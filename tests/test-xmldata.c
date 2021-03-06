/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2012-2015 Matthias Klumpp <matthias@tenstral.net>
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
#include <glib/gprintf.h>

#include "appstream.h"
#include "as-xmldata.h"
#include "as-component-private.h"
#include "as-test-utils.h"

static gchar *datadir = NULL;

void
msg (const gchar *s)
{
	g_printf ("%s\n", s);
}

void
test_screenshot_handling ()
{
	AsMetadata *metad;
	GError *error = NULL;
	AsComponent *cpt;
	GFile *file;
	gchar *path;
	GPtrArray *screenshots;
	guint i;

	metad = as_metadata_new ();
	as_metadata_set_parser_mode (metad, AS_PARSER_MODE_DISTRO);

	path = g_build_filename (datadir, "appstream-dxml.xml", NULL);
	file = g_file_new_for_path (path);
	g_free (path);

	as_metadata_parse_file (metad, file, &error);
	g_object_unref (file);
	g_assert_no_error (error);

	cpt = as_metadata_get_component (metad);
	g_assert (cpt != NULL);

	// dirty...
	g_debug ("%s", as_component_to_string (cpt));
	screenshots = as_component_get_screenshots (cpt);
	g_assert_cmpint (screenshots->len, >, 0);

	for (i = 0; i < screenshots->len; i++) {
		GPtrArray *imgs;
		AsScreenshot *sshot = (AsScreenshot*) g_ptr_array_index (screenshots, i);

		imgs = as_screenshot_get_images (sshot);
		g_assert_cmpint (imgs->len, ==, 2);
		g_debug ("%s", as_screenshot_get_caption (sshot));
	}

	g_object_unref (metad);
}

void
test_appstream_parser_legacy ()
{
	AsMetadata *metad;
	GFile *file;
	gchar *path;
	AsComponent *cpt;
	GPtrArray *screenshots;
	GError *error = NULL;

	metad = as_metadata_new ();

	path = g_build_filename (datadir, "appdata-legacy.xml", NULL);
	file = g_file_new_for_path (path);
	g_free (path);

	as_metadata_parse_file (metad, file, &error);
	cpt = as_metadata_get_component (metad);
	g_object_unref (file);
	g_assert_no_error (error);
	g_assert (cpt != NULL);

	g_assert_cmpstr (as_component_get_summary (cpt), ==, "Application manager for GNOME");
	g_assert (as_component_get_kind (cpt) == AS_COMPONENT_KIND_DESKTOP_APP);

	screenshots = as_component_get_screenshots (cpt);
	g_assert_cmpint (screenshots->len, ==, 5);

	g_object_unref (metad);
}

void
test_appstream_parser_locale ()
{
	g_autoptr(AsMetadata) metad = NULL;
	g_autoptr(GFile) file = NULL;
	gchar *path;
	AsComponent *cpt;

	GPtrArray *trs;
	AsTranslation *tr;

	GError *error = NULL;

	metad = as_metadata_new ();

	path = g_build_filename (datadir, "appdata.xml", NULL);
	file = g_file_new_for_path (path);
	g_free (path);

	/* check german only locale */
	as_metadata_set_locale (metad, "de_DE");
	as_metadata_parse_file (metad, file, &error);
	cpt = as_metadata_get_component (metad);
	g_assert_no_error (error);
	g_assert (cpt != NULL);

	g_assert (as_component_get_kind (cpt) == AS_COMPONENT_KIND_DESKTOP_APP);
	g_assert_cmpstr (as_component_get_name (cpt), ==, "Feuerfuchs");
	as_component_set_active_locale (cpt, "C");
	g_assert_cmpstr (as_component_get_name (cpt), ==, "Firefox");
	/* no french, so fallback */
	as_component_set_active_locale (cpt, "fr_FR");
	g_assert_cmpstr (as_component_get_name (cpt), ==, "Firefox");

	/* check all locale */
	as_metadata_clear_components (metad);
	as_metadata_set_locale (metad, "ALL");
	as_metadata_parse_file (metad, file, &error);
	cpt = as_metadata_get_component (metad);
	g_assert_no_error (error);

	as_component_set_active_locale (cpt, "C");
	g_assert_cmpstr (as_component_get_name (cpt), ==, "Firefox");
	as_component_set_active_locale (cpt, "de_DE");
	g_assert_cmpstr (as_component_get_name (cpt), ==, "Feuerfuchs");
	/* no french, so fallback */
	as_component_set_active_locale (cpt, "fr_FR");
	g_assert_cmpstr (as_component_get_name (cpt), ==, "Firefoux");

	/* check if reading <translation/> tag succeeded */
	trs = as_component_get_translations (cpt);
	g_assert_cmpint (trs->len, ==, 1);
	tr = AS_TRANSLATION (g_ptr_array_index (trs, 0));
	g_assert_cmpstr (as_translation_get_id (tr), ==, "firefox");

	/* check if we loaded the right amount of icons */
	g_assert_cmpint (as_component_get_icons (cpt)->len, ==, 2);
}

void
test_appstream_write_locale ()
{
	AsMetadata *metad;
	GFile *file;
	gchar *tmp;
	AsComponent *cpt;
	GError *error = NULL;

	metad = as_metadata_new ();

	tmp = g_build_filename (datadir, "appdata.xml", NULL);
	file = g_file_new_for_path (tmp);
	g_free (tmp);

	as_metadata_set_locale (metad, "ALL");
	as_metadata_parse_file (metad, file, &error);
	cpt = as_metadata_get_component (metad);
	g_assert_no_error (error);
	g_assert (cpt != NULL);
	g_object_unref (file);

	tmp = as_metadata_component_to_upstream_xml (metad);
	//g_debug ("Generated XML: %s", as_metadata_component_to_upstream_xml (metad));
	g_free (tmp);

	g_object_unref (metad);
}

void
test_appstream_write_description ()
{
	guint i;
	gchar *tmp;
	AsRelease *rel;
	g_autoptr(AsMetadata) metad = NULL;
	g_autoptr(AsComponent) cpt = NULL;

	const gchar *EXPECTED_XML = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
				    "<component>\n"
				    "  <id>org.example.Test</id>\n"
				    "  <name>Test</name>\n"
				    "  <summary>Just a unittest.</summary>\n"
				    "  <description>\n"
				    "    <p>First paragraph</p>\n"
				    "    <ol>\n"
				    "      <li>One</li>\n"
				    "      <li>Two</li>\n"
				    "      <li>Three is &gt; 2 &amp; 1</li>\n"
				    "    </ol>\n"
				    "    <p>Paragraph2</p>\n"
				    "    <ul>\n"
				    "      <li>First</li>\n"
				    "      <li>Second</li>\n"
				    "    </ul>\n"
				    "    <p>Paragraph3 &amp; the last one</p>\n"
				    "  </description>\n"
				    "  <icon type=\"cached\" width=\"20\" height=\"20\">test_writetest.png</icon>\n"
				    "  <icon type=\"cached\" width=\"40\" height=\"40\">test_writetest.png</icon>\n"
				    "  <icon type=\"stock\">xml-writetest</icon>\n"
				    "  <releases>\n"
				    "    <release version=\"1.0\" date=\"2016-04-11T22:00:00Z\"/>\n"
				    "  </releases>\n"
				    "</component>\n";

	const gchar *EXPECTED_XML_LOCALIZED = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
						"<component>\n"
						"  <id>org.example.Test</id>\n"
						"  <name>Test</name>\n"
						"  <summary>Just a unittest.</summary>\n"
						"  <summary xml:lang=\"de\">Nur ein Unittest.</summary>\n"
						"  <description>\n"
						"    <p>First paragraph</p>\n"
						"    <ol>\n"
						"      <li>One</li>\n"
						"      <li>Two</li>\n"
						"      <li>Three is &gt; 2 &amp; 1</li>\n"
						"    </ol>\n"
						"    <p>Paragraph2</p>\n"
						"    <ul>\n"
						"      <li>First</li>\n"
						"      <li>Second</li>\n"
						"    </ul>\n"
						"    <p>Paragraph3 &amp; the last one</p>\n"
						"    <p xml:lang=\"de\">First paragraph</p>\n"
						"    <ol>\n"
						"      <li xml:lang=\"de\">One</li>\n"
						"      <li xml:lang=\"de\">Two</li>\n"
						"      <li xml:lang=\"de\">Three</li>\n"
						"    </ol>\n"
						"    <ul>\n"
						"      <li xml:lang=\"de\">First</li>\n"
						"      <li xml:lang=\"de\">Second</li>\n"
						"    </ul>\n"
						"    <p xml:lang=\"de\">Paragraph2</p>\n"
						"  </description>\n"
						"  <icon type=\"cached\" width=\"20\" height=\"20\">test_writetest.png</icon>\n"
						"  <icon type=\"cached\" width=\"40\" height=\"40\">test_writetest.png</icon>\n"
						"  <icon type=\"stock\">xml-writetest</icon>\n"
						"  <releases>\n"
						"    <release version=\"1.0\" date=\"2016-04-11T22:00:00Z\"/>\n"
						"  </releases>\n"
						"</component>\n";

	const gchar *EXPECTED_XML_DISTRO = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
					   "<components version=\"0.8\">\n"
					   "  <component>\n"
					   "    <id>org.example.Test</id>\n"
					   "    <name>Test</name>\n"
					   "    <summary>Just a unittest.</summary>\n"
					   "    <summary xml:lang=\"de\">Nur ein Unittest.</summary>\n"
					   "    <description>\n"
					   "      <p>First paragraph</p>\n"
					   "      <ol>\n"
					   "        <li>One</li>\n"
					   "        <li>Two</li>\n"
					   "        <li>Three is &gt; 2 &amp; 1</li>\n"
					   "      </ol>\n"
					   "      <p>Paragraph2</p>\n"
					   "      <ul>\n"
					   "        <li>First</li>\n"
					   "        <li>Second</li>\n"
					   "      </ul>\n"
					   "      <p>Paragraph3 &amp; the last one</p>\n"
					   "    </description>\n"
					   "    <description xml:lang=\"de\">\n"
					   "      <p>First paragraph</p>\n"
					   "      <ol>\n"
					   "        <li>One</li>\n"
					   "        <li>Two</li>\n"
					   "        <li>Three</li>\n"
					   "      </ol>\n"
					   "      <ul>\n"
					   "        <li>First</li>\n"
					   "        <li>Second</li>\n"
					   "      </ul>\n"
					   "      <p>Paragraph2</p>\n"
					   "    </description>\n"
					   "    <icon type=\"cached\" width=\"20\" height=\"20\">test_writetest.png</icon>\n"
					   "    <icon type=\"cached\" width=\"40\" height=\"40\">test_writetest.png</icon>\n"
					   "    <icon type=\"stock\">xml-writetest</icon>\n"
					   "    <releases>\n"
					   "      <release version=\"1.0\" timestamp=\"1460412000\"/>\n"
					   "    </releases>\n"
					   "  </component>\n"
					   "</components>\n";

	metad = as_metadata_new ();

	cpt = as_component_new ();
	as_component_set_kind (cpt, AS_COMPONENT_KIND_GENERIC);
	as_component_set_id (cpt, "org.example.Test");
	as_component_set_name (cpt, "Test", "C");
	as_component_set_summary (cpt, "Just a unittest.", "C");
	as_component_set_description (cpt,
				"<p>First paragraph</p>\n<ol><li>One</li><li>Two</li><li>Three is &gt; 2 &amp; 1</li></ol>\n<p>Paragraph2</p><ul><li>First</li><li>Second</li></ul><p>Paragraph3 &amp; the last one</p>",
				NULL);
	rel = as_release_new ();
	as_release_set_version (rel, "1.0");
	as_release_set_timestamp (rel, 1460412000);
	as_component_add_release (cpt, rel);
	g_object_unref (rel);

	for (i = 1; i <= 3; i++) {
		g_autoptr(AsIcon) icon = NULL;

		icon = as_icon_new ();
		if (i != 3)
			as_icon_set_kind (icon, AS_ICON_KIND_CACHED);
		else
			as_icon_set_kind (icon, AS_ICON_KIND_STOCK);
		as_icon_set_width (icon, i * 20);
		as_icon_set_height (icon, i * 20);

		if (i != 3)
			as_icon_set_filename (icon, "test_writetest.png");
		else
			as_icon_set_filename (icon, "xml-writetest");

		as_component_add_icon (cpt, icon);
	}

	as_metadata_add_component (metad, cpt);

	tmp = as_metadata_component_to_upstream_xml (metad);
	g_assert (as_test_compare_lines (tmp, EXPECTED_XML));
	g_free (tmp);

	/* add localization */
	as_component_set_summary (cpt, "Nur ein Unittest.", "de");
	as_component_set_description (cpt,
				"<p>First paragraph</p>\n<ol><li>One</li><li>Two</li><li>Three</li></ol><ul><li>First</li><li>Second</li></ul><p>Paragraph2</p>",
				"de");

	tmp = as_metadata_component_to_upstream_xml (metad);
	g_assert_cmpstr (tmp, ==, EXPECTED_XML_LOCALIZED);
	g_free (tmp);

	tmp = as_metadata_components_to_distro_xml (metad);
	g_assert_cmpstr (tmp, ==, EXPECTED_XML_DISTRO);
	g_free (tmp);
}

AsComponent*
as_xml_test_read_data (const gchar *data, AsParserMode mode)
{
	AsComponent *cpt;
	GError *error = NULL;
	g_autoptr(GPtrArray) cpts = NULL;
	g_autoptr(AsXMLData) xdt = NULL;

	xdt = as_xmldata_new ();
	as_xmldata_set_check_valid (xdt, FALSE);

	if (mode == AS_PARSER_MODE_UPSTREAM) {
		cpt = as_xmldata_parse_upstream_data (xdt, data, &error);
		g_assert_no_error (error);
	} else {
		cpts = as_xmldata_parse_distro_data (xdt, data, &error);
		g_assert_no_error (error);
		cpt = AS_COMPONENT (g_ptr_array_index (cpts, 0));
	}

	return g_object_ref (cpt);
}

gchar*
as_xml_test_serialize (AsComponent *cpt, AsParserMode mode)
{
	gchar *data;
	g_autoptr(AsXMLData) xdt = NULL;

	xdt = as_xmldata_new ();
	as_xmldata_set_check_valid (xdt, FALSE);

	if (mode == AS_PARSER_MODE_UPSTREAM) {
		data = as_xmldata_serialize_to_upstream (xdt, cpt);
	} else {
		g_autoptr(GPtrArray) cpts = NULL;
		cpts = g_ptr_array_new ();
		g_ptr_array_add (cpts, cpt);
		data = as_xmldata_serialize_to_distro (xdt, cpts, TRUE);
	}

	return data;
}

void
test_xml_read_languages (void)
{
	g_autoptr(AsComponent) cpt = NULL;
	const gchar *xmldata_languages = "<component>\n"
					 "  <id>org.example.LangTest</id>\n"
					 "  <languages>\n"
					 "    <lang percentage=\"48\">de_DE</lang>\n"
					 "    <lang percentage=\"100\">en_GB</lang>\n"
					 "  </languages>\n"
					 "</component>\n";

	cpt = as_xml_test_read_data (xmldata_languages, AS_PARSER_MODE_UPSTREAM);
	g_assert_cmpstr (as_component_get_id (cpt), ==, "org.example.LangTest");

	g_assert_cmpint (as_component_get_language (cpt, "de_DE"), ==, 48);
	g_assert_cmpint (as_component_get_language (cpt, "en_GB"), ==, 100);
	g_assert_cmpint (as_component_get_language (cpt, "invalid_C"), ==, -1);
}

void
test_xml_write_languages (void)
{
	g_autoptr(AsComponent) cpt = NULL;
	g_autofree gchar *res = NULL;
	const gchar *expected_lang_xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
					 "<component>\n"
					 "  <id>org.example.LangTest</id>\n"
					 "  <languages>\n"
					 "    <lang percentage=\"86\">de_DE</lang>\n"
					 "    <lang percentage=\"98\">en_GB</lang>\n"
					 "  </languages>\n"
					 "</component>\n";

	cpt = as_component_new ();
	as_component_set_id (cpt, "org.example.LangTest");
	as_component_add_language (cpt, "de_DE", 86);
	as_component_add_language (cpt, "en_GB", 98);

	res = as_xml_test_serialize (cpt, AS_PARSER_MODE_UPSTREAM);
	g_assert_cmpstr (res, ==, expected_lang_xml);
}

void
test_xml_write_releases (void)
{
	g_autoptr(AsComponent) cpt = NULL;
	g_autoptr(AsRelease) rel = NULL;
	g_autofree gchar *res = NULL;
	const gchar *expected_rel_xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
					"<component>\n"
					"  <id>org.example.ReleaseTest</id>\n"
					"  <releases>\n"
					"    <release version=\"1.2\">\n"
					"      <description>\n"
					"        <p>A release description.</p>\n"
					"        <p xml:lang=\"de\">Eine Beschreibung der Veröffentlichung.</p>\n"
					"      </description>\n"
					"    </release>\n"
					"  </releases>\n"
					"</component>\n";

	cpt = as_component_new ();
	as_component_set_id (cpt, "org.example.ReleaseTest");

	rel = as_release_new ();
	as_release_set_version (rel, "1.2");
	as_release_set_description (rel, "<p>A release description.</p>", "C");
	as_release_set_description (rel, "<p>Eine Beschreibung der Veröffentlichung.</p>", "de");

	as_component_add_release (cpt, rel);

	res = as_xml_test_serialize (cpt, AS_PARSER_MODE_UPSTREAM);
	g_assert (as_test_compare_lines (res, expected_rel_xml));
}

void
test_xml_write_provides (void)
{
	g_autoptr(AsComponent) cpt = NULL;
	g_autoptr(AsProvided) prov_mime = NULL;
	g_autoptr(AsProvided) prov_bin = NULL;
	g_autoptr(AsProvided) prov_dbus = NULL;
	g_autofree gchar *res = NULL;
	const gchar *expected_prov_xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
					"<component>\n"
					"  <id>org.example.ProvidesTest</id>\n"
					"  <mimetypes>\n"
					"    <mimetype>image/png</mimetype>\n"
					"    <mimetype>text/plain</mimetype>\n"
					"    <mimetype>application/xml</mimetype>\n"
					"  </mimetypes>\n"
					"  <provides>\n"
					"    <binary>foobar</binary>\n"
					"    <binary>foobar-viewer</binary>\n"
					"    <dbus type=\"system\">org.example.ProvidesTest.Modify</dbus>\n"
					"  </provides>\n"
					"</component>\n";

	cpt = as_component_new ();
	as_component_set_id (cpt, "org.example.ProvidesTest");

	prov_mime = as_provided_new ();
	as_provided_set_kind (prov_mime, AS_PROVIDED_KIND_MIMETYPE);
	as_provided_add_item (prov_mime, "text/plain");
	as_provided_add_item (prov_mime, "application/xml");
	as_provided_add_item (prov_mime, "image/png");
	as_component_add_provided (cpt, prov_mime);

	prov_bin = as_provided_new ();
	as_provided_set_kind (prov_bin, AS_PROVIDED_KIND_BINARY);
	as_provided_add_item (prov_bin, "foobar");
	as_provided_add_item (prov_bin, "foobar-viewer");
	as_component_add_provided (cpt, prov_bin);

	prov_dbus = as_provided_new ();
	as_provided_set_kind (prov_dbus, AS_PROVIDED_KIND_DBUS_SYSTEM);
	as_provided_add_item (prov_dbus, "org.example.ProvidesTest.Modify");
	as_component_add_provided (cpt, prov_dbus);

	res = as_xml_test_serialize (cpt, AS_PARSER_MODE_UPSTREAM);
	g_assert (as_test_compare_lines (res, expected_prov_xml));
}

int
main (int argc, char **argv)
{
	int ret;

	if (argc == 0) {
		g_error ("No test directory specified!");
		return 1;
	}

	datadir = argv[1];
	g_assert (datadir != NULL);
	datadir = g_build_filename (datadir, "samples", NULL);
	g_assert (g_file_test (datadir, G_FILE_TEST_EXISTS) != FALSE);

	g_setenv ("G_MESSAGES_DEBUG", "all", TRUE);
	g_test_init (&argc, &argv, NULL);

	/* only critical and error are fatal */
	g_log_set_fatal_mask (NULL, G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);

	g_test_add_func ("/XML/Screenshots", test_screenshot_handling);
	g_test_add_func ("/XML/LegacyData", test_appstream_parser_legacy);
	g_test_add_func ("/XML/Read/ParserLocale", test_appstream_parser_locale);
	g_test_add_func ("/XML/Write/WriterLocale", test_appstream_write_locale);
	g_test_add_func ("/XML/Write/Description", test_appstream_write_description);
	g_test_add_func ("/XML/Read/Languages", test_xml_read_languages);
	g_test_add_func ("/XML/Write/Languages", test_xml_write_languages);
	g_test_add_func ("/XML/Write/Releases", test_xml_write_releases);
	g_test_add_func ("/XML/Write/Provides", test_xml_write_provides);

	ret = g_test_run ();
	g_free (datadir);
	return ret;
}
