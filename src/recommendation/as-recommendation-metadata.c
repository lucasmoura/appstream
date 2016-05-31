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
 * SECTION:as-recommendation-metadata
 * @short_description: Parser for Recommendation metadata
 * @include: appstream.h
 *
 * This object parses Recommendation metadata.
 * It returns an #AsRecommendation of the data.
 *
 * See also: #AsRecommendation
 */

#include <glib.h>

#include "as-recommendation-metadata.h"
#include "as-recommendation.h"

typedef struct
{
	gchar *origin;

	GPtrArray *recs;
} AsRecommendationMetadataPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (AsRecommendationMetadata, as_recommendation_metadata, G_TYPE_OBJECT)
#define GET_PRIVATE(o) (as_recommendation_metadata_get_instance_private (o))

/**
 * as_recommendation_metadata_init:
 **/
static void
as_recommendation_metadata_init (AsRecommendationMetadata *rec_metad)
{
	AsRecommendationMetadataPrivate *priv = GET_PRIVATE (rec_metad);
	priv->recs = g_ptr_array_new_with_free_func (g_object_unref);
}

/**
 * as_recommendation_metadata_finalize:
 **/
static void
as_recommendation_metadata_finalize (GObject *object)
{
	AsRecommendationMetadata *rec_metad = AS_RECOMMENDATION_METADATA (object);
	AsRecommendationMetadataPrivate *priv = GET_PRIVATE (rec_metad);

	g_ptr_array_unref (priv->recs);
	g_free (priv->origin);

	G_OBJECT_CLASS (as_recommendation_metadata_parent_class)->finalize (object);
}

/**
 * as_recommendation_metadata_set_origin:
 * @rec_metad: an #AsRecommendationMetadata instance.
 * @origin: the origin of the recommendation metadata.
 *
 * Set the origin of the recommendation metadata
 **/
void
as_recommendation_metadata_set_origin (AsRecommendationMetadata *rec_metad,
									   const gchar *origin)
{
	AsRecommendationMetadataPrivate *priv = GET_PRIVATE (rec_metad);
	g_free (priv->origin);
	priv->origin = g_strdup (origin);
}

/**
 * as_recommendation_metadata_get_origin:
 * @rec_metad: an #AsRecommendationMetadata instance.
 *
 * Returns: The origin of the recommendation metadata
 **/
const gchar*
as_recommendation_metadata_get_origin (AsRecommendationMetadata *rec_metad)
{
	AsRecommendationMetadataPrivate *priv = GET_PRIVATE (rec_metad);
	return priv->origin;
}

/**
 * as_recommendation_metadata_add_component:
 *
 * Add an #AsRecommendation to the list of recommendations.
 */
void
as_recommendation_metadata_add_component (AsRecommendationMetadata *rec_metad,
										  AsRecommendation *rec)
{
	AsRecommendationMetadataPrivate *priv = GET_PRIVATE (rec_metad);
	g_ptr_array_add (priv->recs, g_object_ref (rec));
}

/**
 * as_recommendation_metadata_get_components:
 * @rec_metad: a #AsRecommendationMetadata instance.
 *
 * Returns: (transfer none) (element-type AsComponent): A #GPtrArray of all parsed recommendations
 **/
GPtrArray*
as_recommendation_metadata_get_recommendations (AsRecommendationMetadata *rec_metad)
{
	AsRecommendationMetadataPrivate *priv = GET_PRIVATE (rec_metad);
	return priv->recs;
}

/**
 * as_recommendation_metadata_class_init:
 **/
static void
as_recommendation_metadata_class_init (AsRecommendationMetadataClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = as_recommendation_metadata_finalize;
}

/**
 * as_recommendation_metadata_new:
 *
 * Creates a new #AsRecommendationMetadata.
 *
 * Returns: (transfer full): a #AsRecommendationMetadata
 **/
AsRecommendationMetadata*
as_recommendation_metadata_new (void)
{
	AsRecommendationMetadata *rec_metad;
	rec_metad = g_object_new (AS_TYPE_RECOMMENDATION_METADATA, NULL);
	return AS_RECOMMENDATION_METADATA (rec_metad);
}
