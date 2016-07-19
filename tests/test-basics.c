/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2012-2014 Matthias Klumpp <matthias@tenstral.net>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the license, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include "appstream.h"

static gchar *datadir = NULL;

void
test_menuparser ()
{
	g_autoptr(AsMenuParser) parser = NULL;
	g_autoptr(GList) menu_dirs = NULL;
	g_autofree gchar *path = NULL;

	path = g_build_filename (datadir, "categories.xml", NULL);
	parser = as_menu_parser_new_from_file (path);

	menu_dirs = as_menu_parser_parse (parser);
	g_assert (g_list_length (menu_dirs) > 4);
}

void
test_simplemarkup ()
{
	gchar *str;
	str = as_description_markup_convert_simple ("<p>Test!</p><p>Blah.</p><ul><li>A</li><li>B</li></ul><p>End.</p>");
	g_debug ("%s", str);
	g_assert (g_strcmp0 (str, "Test!\n\nBlah.\n • A\n • B\n\nEnd.") == 0);
	g_free (str);
}

gchar**
_get_dummy_strv (const gchar *value)
{
	gchar **strv;

	strv = g_new0 (gchar*, 1 + 2);
	strv[0] = g_strdup (value);
	strv[1] = NULL;

	return strv;
}

void
test_component ()
{
	AsComponent *cpt;
	AsMetadata *metad;
	gchar *str;
	gchar *str2;
	gchar **strv;

	cpt = as_component_new ();
	as_component_set_kind (cpt, AS_COMPONENT_KIND_DESKTOP_APP);

	as_component_set_id (cpt, "org.example.test.desktop");
	as_component_set_name (cpt, "Test", NULL);
	as_component_set_summary (cpt, "It does things", NULL);

	strv = _get_dummy_strv ("fedex");
	as_component_set_pkgnames (cpt, strv);
	g_strfreev (strv);

	metad = as_metadata_new ();
	as_metadata_add_component (metad, cpt);
	str = as_metadata_component_to_upstream_xml (metad);
	str2 = as_metadata_components_to_distro_xml (metad);
	g_object_unref (metad);
	g_debug ("%s", str2);

	g_assert_cmpstr (str, ==, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
				  "<component type=\"desktop\">\n"
				  "  <id>org.example.test.desktop</id>\n"
				  "  <name>Test</name>\n"
				  "  <summary>It does things</summary>\n"
				  "  <pkgname>fedex</pkgname>\n"
				  "</component>\n");
	g_assert_cmpstr (str2, ==, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
				   "<components version=\"0.8\">\n"
				   "  <component type=\"desktop\">\n"
				   "    <id>org.example.test.desktop</id>\n"
				   "    <name>Test</name>\n"
				   "    <summary>It does things</summary>\n"
				   "    <pkgname>fedex</pkgname>\n"
				   "  </component>\n"
				   "</components>\n");

	g_free (str);
	g_free (str2);
}

void
test_merge_components ()
{
	int i;

	g_autoptr(GPtrArray) cpts = NULL;
	g_autoptr(AsDataPool) dpool = NULL;
	g_autoptr(GError) error = NULL;
	g_auto(GStrv) strv = NULL;
	g_auto(GStrv) expected_cpts_id = NULL;
	g_autoptr(GPtrArray) suggestions = NULL;

	GPtrArray* cpts_id = NULL;
	AsComponent *cpt = NULL;
	AsSuggested *suggested = NULL;
	AsSuggestedKind suggested_kind;
	gchar *cpt_id = NULL;

	dpool = as_data_pool_new ();

	strv = g_new0 (gchar*, 2);
	strv[0] = g_build_filename (datadir, "distro", NULL);

	as_data_pool_set_metadata_locations (dpool, strv);
	as_data_pool_load_metadata (dpool);

	cpt = as_data_pool_get_component_by_id (dpool, "test.desktop");
	suggestions = as_component_get_suggestions (cpt);
	suggested = (AsSuggested *) g_ptr_array_index (suggestions, 0);
	cpts_id = as_suggested_get_components_id (suggested);
	suggested_kind = as_suggested_get_kind (suggested);

	expected_cpts_id = g_new0 (gchar*, 3);
	expected_cpts_id[0] = g_strdup ("test1.desktop");
	expected_cpts_id[1] = g_strdup ("test2.desktop");

	g_assert_cmpint (suggestions->len, ==, 2);
	g_assert_cmpint (suggested_kind, ==, AS_SUGGESTED_KIND_HEURISTIC);

	for (i = 0; i < cpts_id->len; i++) {
		cpt_id = (gchar*) g_ptr_array_index (cpts_id, i);
		g_assert_cmpstr (cpt_id, ==, expected_cpts_id[i]);
	}

	cpt = as_data_pool_get_component_by_id (dpool, "test1.desktop");
	suggestions = as_component_get_suggestions (cpt);
	suggested = (AsSuggested *) g_ptr_array_index (suggestions, 0);
	cpts_id = as_suggested_get_components_id (suggested);
	suggested_kind = as_suggested_get_kind (suggested);

	expected_cpts_id = g_new0 (gchar*, 3);
	expected_cpts_id[0] = g_strdup ("test5.desktop");
	expected_cpts_id[1] = g_strdup ("test6.desktop");

	g_assert_cmpint (suggestions->len, ==, 2);
	g_assert_cmpint (suggested_kind, ==, AS_SUGGESTED_KIND_HEURISTIC);

	for (i = 0; i < cpts_id->len; i++) {
		cpt_id = (gchar*) g_ptr_array_index (cpts_id, i);
		g_assert_cmpstr (cpt_id, ==, expected_cpts_id[i]);
	}

}

int
main (int argc, char **argv)
{
	int ret;

	if (argc == 0) {
		g_error ("No test directory specified!");
		return 1;
	}

	g_assert (argv[1] != NULL);
	datadir = g_build_filename (argv[1], "samples", NULL);
	g_assert (g_file_test (datadir, G_FILE_TEST_EXISTS) != FALSE);

	g_setenv ("G_MESSAGES_DEBUG", "all", TRUE);
	g_test_init (&argc, &argv, NULL);

	/* only critical and error are fatal */
	g_log_set_fatal_mask (NULL, G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);

	g_test_add_func ("/AppStream/MenuParser", test_menuparser);
	g_test_add_func ("/AppStream/SimpleMarkupConvert", test_simplemarkup);
	g_test_add_func ("/AppStream/Component", test_component);
	g_test_add_func ("/AppStream/ComponentMerge", test_merge_components);

	ret = g_test_run ();
	g_free (datadir);
	return ret;
}
