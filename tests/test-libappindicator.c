/*
 * Tests for the libappindicator library.
 *
 * Copyright 2009 Ted Gould <ted@canonical.com>
 * Copyright 2023-2024 Robert Tari <robert@tari.in>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ayatana-appindicator.h>

void onStatus (AppIndicator *pIndicator, gchar *sStatus, gboolean *bActivated)
{
    *bActivated = TRUE;
}

void onSignal (AppIndicator *pIndicator, gboolean *bActivated)
{
    *bActivated = TRUE;
}

void propSignals ()
{
    AppIndicator *pIndicator = app_indicator_new ("test-app-indicator", "indicator-messages", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    g_assert (pIndicator != NULL);

    gboolean bSignaled = FALSE;
    gulong nHandler = 0;
    nHandler = g_signal_connect (G_OBJECT (pIndicator), "new-icon", G_CALLBACK (onSignal), &bSignaled);

    g_assert (nHandler != 0);

    nHandler = g_signal_connect (G_OBJECT (pIndicator), "new-attention-icon", G_CALLBACK (onSignal), &bSignaled);

    g_assert (nHandler != 0);

    nHandler = g_signal_connect (G_OBJECT (pIndicator), "new-status", G_CALLBACK (onStatus), &bSignaled);

    g_assert (nHandler != 0);

    bSignaled = FALSE;
    app_indicator_set_icon (pIndicator, "bob", NULL);

    g_assert (bSignaled);

    bSignaled = FALSE;
    app_indicator_set_icon (pIndicator, "bob", NULL);

    g_assert (!bSignaled);

    bSignaled = FALSE;
    app_indicator_set_icon (pIndicator, "al", NULL);

    g_assert (bSignaled);

    bSignaled = FALSE;
    app_indicator_set_attention_icon (pIndicator, "bob", NULL);

    g_assert (bSignaled);

    bSignaled = FALSE;
    app_indicator_set_attention_icon (pIndicator, "bob", NULL);

    g_assert (!bSignaled);

    bSignaled = FALSE;
    app_indicator_set_attention_icon (pIndicator, "al", NULL);

    g_assert (bSignaled);

    bSignaled = FALSE;
    app_indicator_set_status (pIndicator, APP_INDICATOR_STATUS_PASSIVE);

    g_assert (!bSignaled);

    bSignaled = FALSE;
    app_indicator_set_status (pIndicator, APP_INDICATOR_STATUS_ACTIVE);

    g_assert (bSignaled);

    bSignaled = FALSE;
    app_indicator_set_status (pIndicator, APP_INDICATOR_STATUS_ACTIVE);

    g_assert(!bSignaled);

    bSignaled = FALSE;
    app_indicator_set_status (pIndicator, APP_INDICATOR_STATUS_ATTENTION);

    g_assert (bSignaled);
}

void initSetProps ()
{
    AppIndicator *pIndicator = app_indicator_new ("my-id", "my-name", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    g_assert (pIndicator != NULL);
    app_indicator_set_status (pIndicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_attention_icon (pIndicator, "my-attention-name", NULL);
    app_indicator_set_title (pIndicator, "My Title");
    const gchar *sId = app_indicator_get_id (pIndicator);
    gint nCompare = g_strcmp0 ("my-id", sId);

    g_assert (!nCompare);

    const gchar *sIcon = app_indicator_get_icon (pIndicator);
    nCompare = g_strcmp0 ("my-name", sIcon);

    g_assert (!nCompare);

    const gchar *sAttentionIcon = app_indicator_get_attention_icon (pIndicator);
    nCompare = g_strcmp0 ("my-attention-name", sAttentionIcon);

    g_assert (!nCompare);

    const gchar *sTitle = app_indicator_get_title (pIndicator);
    nCompare = g_strcmp0 ("My Title", sTitle);

    g_assert (!nCompare);

    AppIndicatorStatus nStatus = app_indicator_get_status (pIndicator);

    g_assert (nStatus == APP_INDICATOR_STATUS_ACTIVE);

    AppIndicatorCategory nCategory = app_indicator_get_category (pIndicator);

    g_assert (nCategory == APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    g_object_unref (G_OBJECT (pIndicator));
}

void initProps ()
{
    AppIndicator *pIndicator = app_indicator_new ("my-id", "my-name", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_status (pIndicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_attention_icon (pIndicator, "my-attention-name", NULL);

    g_assert (pIndicator != NULL);

    const gchar *sId = app_indicator_get_id (pIndicator);
    gint nCompare = g_strcmp0 ("my-id", sId);

    g_assert (!nCompare);

    const gchar *sIcon = app_indicator_get_icon (pIndicator);
    nCompare = g_strcmp0 ("my-name", sIcon);

    g_assert (!nCompare);

    const gchar *sAttentionIcon = app_indicator_get_attention_icon (pIndicator);
    nCompare = g_strcmp0 ("my-attention-name", sAttentionIcon);

    g_assert (!nCompare);

    AppIndicatorStatus nStatus = app_indicator_get_status (pIndicator);

    g_assert (nStatus == APP_INDICATOR_STATUS_ACTIVE);

    AppIndicatorCategory nCategory = app_indicator_get_category (pIndicator);

    g_assert (nCategory == APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    g_object_unref (G_OBJECT (pIndicator));
}

void init ()
{
    AppIndicator *pIndicator = app_indicator_new ("my-id", "my-name", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    g_assert (pIndicator != NULL);

    g_object_unref (G_OBJECT (pIndicator));
}

void setLabel ()
{
    AppIndicator *pIndicator = app_indicator_new ("my-id", "my-name", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    g_assert (pIndicator != NULL);

    const gchar *sLabel = app_indicator_get_label (pIndicator);

    g_assert (sLabel == NULL);

    const gchar *sLabelGuide = app_indicator_get_label_guide (pIndicator);

    g_assert (sLabelGuide == NULL);

    // First check all the clearing modes, this is important as we're going to use them later, we need them to work.
    app_indicator_set_label (pIndicator, NULL, NULL);
    sLabel = app_indicator_get_label (pIndicator);

    g_assert (sLabel == NULL);

    sLabelGuide = app_indicator_get_label_guide (pIndicator);

    g_assert (sLabelGuide == NULL);

    app_indicator_set_label (pIndicator, "", NULL);
    sLabel = app_indicator_get_label (pIndicator);

    g_assert (sLabel == NULL);

    sLabelGuide = app_indicator_get_label_guide (pIndicator);

    g_assert (sLabelGuide == NULL);

    app_indicator_set_label (pIndicator, NULL, "");
    sLabel = app_indicator_get_label (pIndicator);

    g_assert (sLabel == NULL);

    sLabelGuide = app_indicator_get_label_guide (pIndicator);

    g_assert (sLabelGuide == NULL);

    app_indicator_set_label (pIndicator, "", "");
    sLabel = app_indicator_get_label (pIndicator);

    g_assert (sLabel == NULL);

    sLabelGuide = app_indicator_get_label_guide (pIndicator);

    g_assert(sLabelGuide == NULL);

    app_indicator_set_label (pIndicator, "label", "");

    sLabel = app_indicator_get_label (pIndicator);
    gint nCompare = g_strcmp0 (sLabel, "label");

    g_assert (nCompare == 0);

    sLabelGuide = app_indicator_get_label_guide (pIndicator);

    g_assert (sLabelGuide == NULL);

    app_indicator_set_label (pIndicator, NULL, NULL);
    sLabel = app_indicator_get_label (pIndicator);

    g_assert (sLabel == NULL);

    sLabelGuide = app_indicator_get_label_guide (pIndicator);

    g_assert(sLabelGuide == NULL);

    app_indicator_set_label (pIndicator, "label", "guide");

    sLabel = app_indicator_get_label (pIndicator);
    nCompare = g_strcmp0 (sLabel, "label");

    g_assert (nCompare == 0);

    sLabelGuide = app_indicator_get_label_guide (pIndicator);
    nCompare = g_strcmp0 (sLabelGuide, "guide");

    g_assert (nCompare == 0);

    app_indicator_set_label (pIndicator, "label2", "guide");

    sLabel = app_indicator_get_label (pIndicator);
    nCompare = g_strcmp0 (sLabel, "label2");

    g_assert (nCompare == 0);

    sLabelGuide = app_indicator_get_label_guide (pIndicator);
    nCompare = g_strcmp0 (sLabelGuide, "guide");

    g_assert (nCompare == 0);

    app_indicator_set_label (pIndicator, "trick-label", "trick-guide");

    sLabel = app_indicator_get_label (pIndicator);
    nCompare = g_strcmp0 (sLabel, "trick-label");

    g_assert (nCompare == 0);

    sLabelGuide = app_indicator_get_label_guide (pIndicator);
    nCompare = g_strcmp0 (sLabelGuide, "trick-guide");

    g_assert (nCompare == 0);

    g_object_unref (G_OBJECT (pIndicator));
}

void onLabelSignals (AppIndicator *pIndicator, gchar *sLabel, gchar *sGuide, gpointer pData)
{
    gint *nCount = (gint*) pData;
    (*nCount)++;
}

void labelSignalsCheck ()
{
    while (g_main_context_pending (NULL))
    {
        g_main_context_iteration (NULL, TRUE);
    }
}

void labelSignals ()
{
    gint nLabelSignals = 0;
    AppIndicator *pIndicator = app_indicator_new ("my-id", "my-name", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    g_assert (pIndicator != NULL);

    const gchar *sLabel = app_indicator_get_label (pIndicator);

    g_assert (sLabel == NULL);

    const gchar *sLabelGuide = app_indicator_get_label_guide (pIndicator);

    g_assert (sLabelGuide == NULL);

    g_signal_connect (G_OBJECT (pIndicator), "new-label", G_CALLBACK (onLabelSignals), &nLabelSignals);

    // Shouldn't be a signal as it should be stuck in idle
    app_indicator_set_label (pIndicator, "label", "guide");

    g_assert (nLabelSignals == 0);

    // Should show up after idle processing
    labelSignalsCheck ();
    g_assert (nLabelSignals == 1);

    // Shouldn't signal with no change
    nLabelSignals = 0;
    app_indicator_set_label (pIndicator, "label", "guide");
    labelSignalsCheck ();

    g_assert (nLabelSignals == 0);

    // Change one, we should get one signal
    app_indicator_set_label (pIndicator, "label2", "guide");
    labelSignalsCheck ();

    g_assert (nLabelSignals == 1);

    // Change several times, one signal
    nLabelSignals = 0;
    app_indicator_set_label (pIndicator, "label1", "guide0");
    app_indicator_set_label (pIndicator, "label1", "guide1");
    app_indicator_set_label (pIndicator, "label2", "guide2");
    app_indicator_set_label (pIndicator, "label3", "guide3");
    labelSignalsCheck ();

    g_assert (nLabelSignals == 1);

    // Clear should signal too
    nLabelSignals = 0;
    app_indicator_set_label (pIndicator, NULL, NULL);
    labelSignalsCheck ();

    g_assert (nLabelSignals == 1);
}

gint main (gint argc, gchar * argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/indicator-application/libappindicator/init", init);
    g_test_add_func ("/indicator-application/libappindicator/init_props", initProps);
    g_test_add_func ("/indicator-application/libappindicator/init_set_props", initSetProps);
    g_test_add_func ("/indicator-application/libappindicator/prop_signals", propSignals);
    g_test_add_func ("/indicator-application/libappindicator/set_label", setLabel);
    g_test_add_func ("/indicator-application/libappindicator/label_signals", labelSignals);

    return g_test_run ();
}
