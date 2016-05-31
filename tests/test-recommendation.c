/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2016 Lucas Moura <lucas.moura128@gmail.com>
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

void
test_recommendation ()
{
	AsRecommendation *rec;
	gchar *recommendation;
	gchar **because;

	rec = as_recommendation_new ();

	as_recommendation_set_recommended(rec, "org.example.test.desktop");

	because = g_new0 (gchar*, 4);
	because[0] = g_strdup ("test1");
	because[1] = g_strdup ("test2");

	as_recommendation_set_because (rec, because);
	g_strfreev (because);

	recommendation = (gchar*) as_recommendation_get_recommended (rec);
	because = as_recommendation_get_because (rec);

	g_assert_cmpstr (recommendation, ==, "org.example.test.desktop");
	g_assert_cmpstr (because[0], ==, "test1");
	g_assert_cmpstr (because[1], ==, "test2");

	g_free (recommendation);
	g_strfreev (because);
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

	g_setenv ("G_MESSAGES_DEBUG", "all", TRUE);
	g_test_init (&argc, &argv, NULL);

	/* only critical and error are fatal */
	g_log_set_fatal_mask (NULL, G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);

	g_test_add_func ("/AppStream/Recommendation", test_recommendation);

	ret = g_test_run ();
	return ret;
}
