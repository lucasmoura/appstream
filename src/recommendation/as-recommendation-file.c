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

/**
 * SECTION:as-recommendation-file
 * @short_description: AppStream recommendation file reading.
 * @include: appstream.h
 *
 * Class to read an AppStream recommendation file.
 * This class is used by #AsRecommendationMetadata to read
 * AppStream recommendation files.
 *
 * See also: #AsRecommendation, #AsRecommendationMetadata
 */

#include <libxml/parser.h>
#include <libxml/xmlsave.h>
#include <string.h>

#include "as-recommendation-file.h"
#include "as-recommendation.h"

#include "../as-utils-private.h"
#include "../as-metadata.h"

typedef struct
{
	gchar *origin;
	gboolean check_valid;

	gchar *last_error_msg;
} AsRecommendationFilePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AsRecommendationFile, as_recommendation_file, G_TYPE_OBJECT)
#define GET_PRIVATE(o) (as_recommendation_file_get_instance_private (o))

static void
libxml_generic_error (AsRecommendationFile *rec_file, const char *format, ...)
{
	GString *str;
	va_list arg_ptr;
	static GMutex mutex;
	AsRecommendationFilePrivate *priv = GET_PRIVATE (rec_file);

	g_mutex_lock (&mutex);
	str = g_string_new (priv->last_error_msg? priv->last_error_msg : "");

	va_start (arg_ptr, format);
	g_string_append_vprintf (str, format, arg_ptr);
	va_end (arg_ptr);

	g_free (priv->last_error_msg);
	priv->last_error_msg = g_string_free (str, FALSE);
	g_mutex_unlock (&mutex);
}

gchar**
as_recommendation_file_get_children_as_strv (AsRecommendationFile *rec_file, xmlNode* node,
											 const gchar* element_name)
{
	GPtrArray *list;
	xmlNode *iter;
	gchar **res;
	g_return_val_if_fail (rec_file != NULL, NULL);
	g_return_val_if_fail (element_name != NULL, NULL);
	list = g_ptr_array_new_with_free_func (g_free);

	for (iter = node->children; iter != NULL; iter = iter->next) {
		/* discard spaces */
		if (iter->type != XML_ELEMENT_NODE) {
					continue;
		}
		if (g_strcmp0 ((gchar*) iter->name, element_name) == 0) {
			gchar* content = NULL;
			content = (gchar*) xmlNodeGetContent (iter);
			if (content != NULL) {
				gchar *s;
				s = g_strdup (content);
				g_strstrip (s);
				g_ptr_array_add (list, s);
			}
			g_free (content);
		}
	}

	res = as_ptr_array_to_strv (list);
	g_ptr_array_unref (list);
	return res;
}

/**
 * as_recommendation_file_init:
 **/
static void
as_recommendation_file_init (AsRecommendationFile *rec_file)
{
	AsRecommendationFilePrivate *priv = GET_PRIVATE (rec_file);

	priv->last_error_msg = NULL;
	priv->check_valid = TRUE;
}

/**
 * as_recommendation_file_finalize:
 **/
static void
as_recommendation_file_finalize (GObject *object)
{
	AsRecommendationFile *rec_file = AS_RECOMMENDATION_FILE (object);
	AsRecommendationFilePrivate *priv = GET_PRIVATE (rec_file);

	g_free (priv->origin);
	g_free (priv->last_error_msg);
	xmlSetGenericErrorFunc (rec_file, NULL);

	G_OBJECT_CLASS (as_recommendation_file_parent_class)->finalize (object);
}

/**
 * as_recommendation_file_clear_error:
 */
static void
as_recommendation_file_clear_error (AsRecommendationFile *rec_file)
{
	static GMutex mutex;
	AsRecommendationFilePrivate *priv = GET_PRIVATE (rec_file);

	g_mutex_lock (&mutex);
	g_free (priv->last_error_msg);
	priv->last_error_msg = NULL;
	xmlSetGenericErrorFunc (rec_file, (xmlGenericErrorFunc) libxml_generic_error);
	g_mutex_unlock (&mutex);
}

/**
 * as_recommendation_file_initialize:
 * @rec_file: An instance of #AsRecommendationFile
 *
 * Initialize the Recommendation file handler.
 */
void
as_recommendation_file_initialize (AsRecommendationFile *rec_file, const gchar *origin)
{
	AsRecommendationFilePrivate *priv = GET_PRIVATE (rec_file);

	g_free (priv->origin);
	priv->origin = g_strdup (origin);

	as_recommendation_file_clear_error (rec_file);
}

/**
 * as_recommendation_file_get_node_value:
 */
static gchar*
as_recommendation_file_get_node_value (AsRecommendationFile *rec_file, xmlNode *node)
{
	gchar *content;
	content = (gchar*) xmlNodeGetContent (node);

	return content;
}

/**
 * as_recommendation_file_parse_recommendation_node:
 */
void
as_recommendation_file_parse_recommendation_node (AsRecommendationFile *rec_file, xmlNode* node,
												  AsRecommendation *rec, GError **error)
{
	xmlNode *iter;
	const gchar *node_name;
	AsRecommendationFilePrivate *priv = GET_PRIVATE (rec_file);

	for (iter = node->children; iter != NULL; iter = iter->next) {
		g_autofree gchar *content = NULL;

		/* discard spaces */
		if (iter->type != XML_ELEMENT_NODE)
			continue;

		node_name = (const gchar*) iter->name;
		content = as_recommendation_file_get_node_value (rec_file, iter);
		g_strstrip (content);

		if (g_strcmp0 (node_name, "recommended") == 0) {
				as_recommendation_set_recommended (rec, content);
		} else if (g_strcmp0 (node_name, "because") == 0) {
			gchar **because_array;
			because_array = as_recommendation_file_get_children_as_strv (rec_file, iter, "pkg");
			as_recommendation_set_because (rec, because_array);
			g_strfreev (because_array);
		}
	}

}

/**
 * as_recommendation_file_parse_components_node:
 */
void
as_recommendation_file_parse_recommendations_node (AsRecommendationFile *rec_file,
												   GPtrArray *recs, xmlNode* node, GError **error)
{
	xmlNode* iter;
	GError *tmp_error = NULL;
	AsRecommendationFilePrivate *priv = GET_PRIVATE (rec_file);

	/* set origin of this metadata */
	g_free (priv->origin);
	priv->origin = (gchar*) xmlGetProp (node, (xmlChar*) "origin");

	for (iter = node->children; iter != NULL; iter = iter->next) {
		/* discard spaces */
		if (iter->type != XML_ELEMENT_NODE)
			continue;

		if (g_strcmp0 ((gchar*) iter->name, "recommendation") == 0) {
			g_autoptr(AsRecommendation) rec = NULL;

			rec = as_recommendation_new ();
			as_recommendation_file_parse_recommendation_node (rec_file, iter, rec, &tmp_error);
			if (tmp_error != NULL) {
				g_propagate_error (error, tmp_error);
				return;
			} else {
				g_ptr_array_add (recs, g_object_ref (rec));
			}
		}
	}
}

/**
 * as_recommendation_parse_document:
 */
xmlDoc*
as_recommendation_file_parse_document (AsRecommendationFile *rec_file, const gchar *data, GError **error)
{
	xmlDoc *doc;
	xmlNode *root;
	AsRecommendationFilePrivate *priv = GET_PRIVATE (rec_file);

	if (data == NULL)
		return NULL;


	as_recommendation_file_clear_error (rec_file);
	doc = xmlReadMemory (data, strlen (data),
			     NULL,
			     "utf-8",
			     XML_PARSE_NOBLANKS | XML_PARSE_NONET);
	if (doc == NULL) {
		g_set_error (error,
				AS_METADATA_ERROR,
				AS_METADATA_ERROR_FAILED,
				"Could not parse XML data: %s", priv->last_error_msg);
		return NULL;
	}

	root = xmlDocGetRootElement (doc);
	if (root == NULL) {
		g_set_error_literal (error,
				     AS_METADATA_ERROR,
				     AS_METADATA_ERROR_FAILED,
				     "The XML document is empty.");
		xmlFreeDoc (doc);
		return NULL;
	}

	return doc;
}

/**
 * as_recommendation_file_parse_distro_data:
 * @rec_file: An instance of #AsRecommendationFile
 * @data: XML representing a recommendation file
 * @error: A #GError
 *
 * Parse AppStream recommendation field.
 *
 * Returns: (transfer full) (element-type AsRecommendation): An array of #AsRecommendation,
 *			deserialized from the XML.
 */
GPtrArray*
as_recommendation_file_parse_recommendation_file (AsRecommendationFile *rec_file,
												  const gchar *data, GError **error)
{
	xmlDoc *doc;
	xmlNode *root;
	GPtrArray *recs = NULL;
	AsRecommendationFilePrivate *priv = GET_PRIVATE (rec_file);

	doc = as_recommendation_file_parse_document (rec_file, data, error);
	if (doc == NULL)
		return NULL;
	root = xmlDocGetRootElement (doc);

	recs = g_ptr_array_new_with_free_func (g_object_unref);

	if (g_strcmp0 ((gchar*) root->name, "recommendations") == 0) {
		as_recommendation_file_parse_recommendations_node (rec_file, recs, root, error);
	} else if (g_strcmp0 ((gchar*) root->name, "recommendation") == 0) {
		g_autoptr(AsRecommendation) rec = NULL;

		rec = as_recommendation_new ();
		as_recommendation_file_parse_recommendation_node (rec_file, root, rec, error);
		g_ptr_array_add (recs, g_object_ref (rec));
	} else {
		g_set_error_literal (error,
					AS_METADATA_ERROR,
					AS_METADATA_ERROR_FAILED,
					"XML file does not contain valid AppStream data!");
		goto out;
	}
	

out:
	xmlFreeDoc (doc);
	return recs;
}

/**
 * as_recommendation_file_class_init:
 **/
static void
as_recommendation_file_class_init (AsRecommendationFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = as_recommendation_file_finalize;
}

/**
 * as_recommendation_file_new:
 */
AsRecommendationFile*
as_recommendation_file_new (void)
{
	AsRecommendationFile *rec_file;
	rec_file = g_object_new (AS_TYPE_RECOMMENDATION_FILE, NULL);
	return AS_RECOMMENDATION_FILE (rec_file);
}
