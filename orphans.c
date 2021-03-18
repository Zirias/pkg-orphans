#include <pkg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ONAME "orphans"
#define OVERS "0.1"
#define ODESC "Find orphaned packages"

static void usage(void)
{
    fprintf(stderr, "Usage: pkg orphans [-U]\n"
	    "\t\t-U\tskip updating repositories\n");
}

static int orphans(int argc, char **argv)
{
    if (argc > 2)
    {
	usage();
	return EXIT_FAILURE;
    }
    int doupdate = 1;
    if (argc == 2)
    {
	if (strcmp(argv[1], "-U"))
	{
	    usage();
	    return EXIT_FAILURE;
	}
	doupdate = 0;
    }

    if (pkgdb_access(PKGDB_MODE_READ|PKGDB_MODE_WRITE|PKGDB_MODE_CREATE,
		PKGDB_DB_REPO) == EPKG_ENOACCESS)
    {
	doupdate = 0;
    }

    if (doupdate)
    {
	if (pkg_repos_total_count() == 0) return EPKG_OK;

	struct pkg_repo *r = 0;
	while (pkg_repos(&r) == EPKG_OK)
	{
	    if (pkg_repo_enabled(r))
	    {
		fprintf(stderr, "Updating %s repository catalogue...\n",
			pkg_repo_name(r));
		pkg_update(r, 0);
	    }
	}
    }

    int rc = EPKG_OK;
    struct pkgdb *db = 0;
    if (pkgdb_open_all(&db, PKGDB_REMOTE, 0) < 0)
    {
	fprintf(stderr, "Cannot open database.");
	return EPKG_ENODB;
    }

    if (pkgdb_obtain_lock(db, PKGDB_LOCK_READONLY) < 0)
    {
	fprintf(stderr, "Cannot obtain read lock on database.");
	rc = EPKG_LOCKED;
	goto done;
    }

    struct pkgdb_it *lit = pkgdb_query(db, 0, MATCH_ALL);
    if (lit)
    {
	struct pkg *p;
	struct pkg *rp;
	pkg_new(&p, PKG_NONE);
	pkg_new(&rp, PKG_NONE);
	const char *name;
	while (pkgdb_it_next(lit, &p, PKG_LOAD_BASIC) == 0)
	{
	    pkg_get(p, PKG_NAME, &name);
	    struct pkgdb_it *rit = pkgdb_repo_query(db, name, MATCH_EXACT, 0);
	    int found = 0;
	    if (rit)
	    {
		if (pkgdb_it_next(rit, &rp, PKG_LOAD_BASIC) == 0) found = 1;
		pkgdb_it_free(rit);
	    }
	    if (!found)
	    {
		puts(name);
	    }
	}
	pkg_free(rp);
	pkg_free(p);
	pkgdb_it_free(lit);
    }

    pkgdb_release_lock(db, PKGDB_LOCK_READONLY);
done:
    pkgdb_close(db);
    return rc;
}

int pkg_plugin_init(struct pkg_plugin *p)
{
    pkg_plugin_set(p, PKG_PLUGIN_NAME, ONAME);
    pkg_plugin_set(p, PKG_PLUGIN_VERSION, OVERS);
    pkg_plugin_set(p, PKG_PLUGIN_DESC, ODESC);

    return EPKG_OK;
}

int pkg_register_cmd(int idx, const char **name, const char **desc,
	int (**exec)(int argc, char **argv))
{
    *name = ONAME;
    *desc = ODESC;
    *exec = orphans;

    return EPKG_OK;
}

int pkg_register_cmd_count(void)
{
    return 1;
}
