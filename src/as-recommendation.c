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

#include "as-recommendation.h"

#include <glib.h>
#include <stdio.h>
#include <glib-object.h>


/**
 * SECTION:as-recommedantion
 * @short_description: Object represeting a package recommendation
 * @include: appstream.h
 *
 * This object represents an Appstream package recommendation which is
 * associated to an Appstream software component.
 * A recommendation can only be any kind of a compenent, except a font or codec.
 *
 */

typedef struct
{
	gchar	  *recommended;
	gchar	  **because;
} AsRecommendationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AsRecommendation, as_recommendation, G_TYPE_OBJECT)
#define GET_PRIVATE(o) (as_recommendation_get_instance_private (o))

enum {
	AS_RECOMMENDATION_DUMMY_PROPERTY,
	AS_RECOMMENDATION_RECOMMENDED_PACKAGE,
	AS_RECOMMENDATION_BECAUSE
};

/**
 * as_recommendation_finalize:
 */
static void
as_recommendation_finalize (GObject* object)
{
	AsRecommendation *rec = AS_RECOMMENDATION (object);
	AsRecommendationPrivate *priv = GET_PRIVATE (rec);

	g_free (priv->recommended);
	g_strfreev (priv->because);

	G_OBJECT_CLASS (as_recommendation_parent_class)->finalize (object);
}

/**
 * as_recommendation_get_recommended:
 * @rec: a #AsRecommendation instance.
 *
 * Returns: The recommended package name.
 */
const gchar*
as_recommendation_get_recommended (AsRecommendation *rec)
{
	AsRecommendationPrivate *priv = GET_PRIVATE (rec);
	return priv->recommended;
}

/**
 * as_recommendation_set_recommended
 * @rec: a #AsRecommendation instance.
 * @value: the recommended package name.
 *
 * Set the recommended package name for this recommendation.
 */
void
as_recommendation_set_recommended(AsRecommendation *rec, const gchar* value)
{
	AsRecommendationPrivate *priv = GET_PRIVATE (rec);

	g_free (priv->recommended);
	priv->recommended = g_strdup (value);
	g_object_notify ((GObject *) rec, "recommended");
}

/**
 * as_recommendation_get_because:
 * @rec: a #AsRecommendation instance.
 *
 * Get a list of package names which generated the recommend package.
 *
 * Returns: (transfer none): String array of package names.
 */
gchar**
as_recommendation_get_because (AsRecommendation *rec)
{
	AsRecommendationPrivate *priv = GET_PRIVATE (rec);
	return priv->because;
}

/**
 * as_recommendation_set_because:
 * @rec: a #AsRecommendation instance.
 * @value: (array zero-terminated=1)
 *
 * Set a list of package names that generated the recommended package.
 */
void
as_recommendation_set_because (AsRecommendation *rec, gchar** value)
{
	AsRecommendationPrivate *priv = GET_PRIVATE (rec);

	g_strfreev (priv->because);
	priv->because = g_strdupv (value);
	g_object_notify ((GObject *) rec, "because");
}

/**
 * as_recommendation_get_property:
 */
static void
as_recommendation_get_property (GObject * object, guint property_id, GValue * value, GParamSpec *pspec)
{
	AsRecommendation *rec;
	rec = G_TYPE_CHECK_INSTANCE_CAST (object, AS_TYPE_RECOMMENDATION, AsRecommendation);

	switch(property_id) {
		case AS_RECOMMENDATION_RECOMMENDED_PACKAGE:
			g_value_set_string (value, as_recommendation_get_recommended (rec));
			break;
		case AS_RECOMMENDATION_BECAUSE:
			g_value_set_boxed (value, as_recommendation_get_because (rec));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

/**
 * as_recommendation_set_property:
 */
static void
as_recommendation_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec)
{
	AsRecommendation *rec;
	rec = G_TYPE_CHECK_INSTANCE_CAST (object, AS_TYPE_RECOMMENDATION, AsRecommendation);

	switch(property_id) {
		case AS_RECOMMENDATION_RECOMMENDED_PACKAGE:
			as_recommendation_set_recommended (rec, g_value_get_string (value));
			break;
		case AS_RECOMMENDATION_BECAUSE:
			as_recommendation_set_because (rec, g_value_get_boxed (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

/**
 * as_recommendation_init:
 */
static void
as_recommendation_init (AsRecommendation *rec)
{
}

/**
 * as_recommendation_class_init
 */
static void
as_recommendation_class_init (AsRecommendationClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = as_recommendation_finalize;
	object_class->get_property = as_recommendation_get_property;
	object_class->set_property = as_recommendation_set_property;

	/**
	 * AsRecommendation:recommended:
	 *
	 * the name of the recommendation package
	 */
	g_object_class_install_property (object_class,
					AS_RECOMMENDATION_RECOMMENDED_PACKAGE,
					g_param_spec_string("recommended", "recommended", "recommended", NULL, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));

	/**
	 * AsRecommendation:because:
	 *
	 * string array of package names that generated the recommendation
	 */
	g_object_class_install_property(object_class,
					AS_RECOMMENDATION_BECAUSE,
					g_param_spec_boxed("because", "because", "because", G_TYPE_STRV, G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_WRITABLE));

}

/**
 * as_recommendation_new:
 *
 * Creates a new #AsRecommendation.
 *
 * Returns: (transfer full): a new #AsRecommendation
 */
AsRecommendation*
as_recommendation_new (void)
{
	AsRecommendation *rec;
	rec = g_object_new (AS_TYPE_RECOMMENDATION, NULL);
	return AS_RECOMMENDATION (rec);
}
