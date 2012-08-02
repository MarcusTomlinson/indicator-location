
#include <glib/gi18n.h>

#include <libappindicator/app-indicator.h>

#include <geoclue/geoclue-master.h>
#include <geoclue/geoclue-master-client.h>

/* Base variables */
AppIndicator * indicator = NULL;
GMainLoop * mainloop = NULL;

/* Menu items */
GtkMenuItem * accuracy_item = NULL;

/* Geoclue trackers */
static GeoclueMasterClient * geo_master = NULL;
static GeoclueAddress * geo_address = NULL;

/* Prototypes */
static void geo_client_invalid (GeoclueMasterClient * client, gpointer user_data);
static void geo_address_change (GeoclueMasterClient * client, gchar * a, gchar * b, gchar * c, gchar * d, gpointer user_data);
static void geo_create_client (GeoclueMaster * master, GeoclueMasterClient * client, gchar * path, GError * error, gpointer user_data);


/* Update accuracy */
static void
update_accuracy (GeoclueAccuracyLevel level)
{
	const char * icon = NULL;
	const char * icon_desc = NULL;
	const char * item_text = NULL;
	
	switch (level) {
	case GEOCLUE_ACCURACY_LEVEL_NONE:
		icon = "indicator-location-unknown";
		icon_desc = _("Location accuracy unknown");
		item_text = _("Accuracy: Unknown");
		break;
	case GEOCLUE_ACCURACY_LEVEL_COUNTRY:
	case GEOCLUE_ACCURACY_LEVEL_REGION:
	case GEOCLUE_ACCURACY_LEVEL_LOCALITY:
		icon = "indicator-location-region";
		icon_desc = _("Location regional accuracy");
		item_text = _("Accuracy: Regional");
		break;
	case GEOCLUE_ACCURACY_LEVEL_POSTALCODE:
	case GEOCLUE_ACCURACY_LEVEL_STREET:
		icon = "indicator-location-neighborhood";
		icon_desc = _("Location neighborhood accuracy");
		item_text = _("Accuracy: Neighborhood");
		break;
	case GEOCLUE_ACCURACY_LEVEL_DETAILED:
		icon = "indicator-location-specific";
		icon_desc = _("Location specific accuracy");
		item_text = _("Accuracy: Detailed");
		break;
	default:
		g_assert_not_reached();
	}

	if (indicator != NULL) {
		app_indicator_set_icon_full(indicator, icon, icon_desc);
	}

	if (accuracy_item != NULL) {
		gtk_menu_item_set_label(accuracy_item, item_text);
	}

	return;
}

/* Callback from getting the address */
static void
geo_address_cb (GeoclueAddress * address, int timestamp, GHashTable * addy_data, GeoclueAccuracy * accuracy, GError * error, gpointer user_data)
{
	if (error != NULL) {
		g_warning("Unable to get Geoclue address: %s", error->message);
		g_clear_error (&error);
		return;
	}

	GeoclueAccuracyLevel level = GEOCLUE_ACCURACY_LEVEL_NONE;
	geoclue_accuracy_get_details(accuracy, &level, NULL, NULL);
	update_accuracy(level);

	return;
}

/* Clean up the reference we kept to the address and make sure to
   drop the signals incase someone else has one. */
static void
geo_address_clean (void)
{
	if (geo_address == NULL) {
		return;
	}

	g_signal_handlers_disconnect_by_func(G_OBJECT(geo_address), geo_address_cb, NULL);
	g_object_unref(G_OBJECT(geo_address));

	geo_address = NULL;

	return;
}

/* Clean up and remove all signal handlers from the client as we
   unreference it as well. */
static void
geo_client_clean (void)
{
	if (geo_master == NULL) {
		return;
	}

	g_signal_handlers_disconnect_by_func(G_OBJECT(geo_master), geo_client_invalid, NULL);
	g_signal_handlers_disconnect_by_func(G_OBJECT(geo_master), geo_address_change, NULL);
	g_object_unref(G_OBJECT(geo_master));

	geo_master = NULL;

	return;
}

/* Callback from creating the address */
static void
geo_create_address (GeoclueMasterClient * master, GeoclueAddress * address, GError * error, gpointer user_data)
{
	if (error != NULL) {
		g_warning("Unable to create GeoClue address: %s", error->message);
		g_clear_error (&error);
		return;
	}

	/* We shouldn't have created a new address if we already had one
	   so this is a warning.  But, it really is only a mem-leak so we
	   don't need to error out. */
	g_warn_if_fail(geo_address == NULL);
	geo_address_clean();

	g_debug("Created Geoclue Address");
	geo_address = address;
	g_object_ref(G_OBJECT(geo_address));

	geoclue_address_get_address_async(geo_address, geo_address_cb, NULL);

	g_signal_connect(G_OBJECT(address), "address-changed", G_CALLBACK(geo_address_cb), NULL);

	return;
}

/* Callback from setting requirements */
static void
geo_req_set (GeoclueMasterClient * master, GError * error, gpointer user_data)
{
	if (error != NULL) {
		g_warning("Unable to set Geoclue requirements: %s", error->message);
		g_clear_error (&error);
	}
	return;
}

/* Client is killing itself rather oddly */
static void
geo_client_invalid (GeoclueMasterClient * client, gpointer user_data)
{
	g_warning("Master client invalid, rebuilding.");

	/* Client changes we can assume the address is now invalid so we
	   need to unreference the one we had. */
	geo_address_clean();

	/* And our master client is invalid */
	geo_client_clean();

	GeoclueMaster * master = geoclue_master_get_default();
	geoclue_master_create_client_async(master, geo_create_client, NULL);

	update_accuracy(GEOCLUE_ACCURACY_LEVEL_NONE);

	return;
}

/* Address provider changed, we need to get that one */
static void
geo_address_change (GeoclueMasterClient * client, gchar * a, gchar * b, gchar * c, gchar * d, gpointer user_data)
{
	g_warning("Address provider changed.  Let's change");

	/* If the address is supposed to have changed we need to drop the old
	   address before starting to get the new one. */
	geo_address_clean();

	geoclue_master_client_create_address_async(geo_master, geo_create_address, NULL);

	update_accuracy(GEOCLUE_ACCURACY_LEVEL_NONE);

	return;
}

/* Callback from creating the client */
static void
geo_create_client (GeoclueMaster * master, GeoclueMasterClient * client, gchar * path, GError * error, gpointer user_data)
{
	g_debug("Created Geoclue client at: %s", path);

	geo_master = client;

	if (error != NULL) {
		g_warning("Unable to get a GeoClue client!  '%s'  Geolocation based timezone support will not be available.", error->message);
		g_clear_error (&error);
		return;
	}

	if (geo_master == NULL) {
		g_warning(_("Unable to get a GeoClue client!  Geolocation based timezone support will not be available."));
		return;
	}

	g_object_ref(G_OBJECT(geo_master));

	/* New client, make sure we don't have an address hanging on */
	geo_address_clean();

	geoclue_master_client_set_requirements_async(geo_master,
	                                             GEOCLUE_ACCURACY_LEVEL_REGION,
	                                             0,
	                                             FALSE,
	                                             GEOCLUE_RESOURCE_ALL,
	                                             geo_req_set,
	                                             NULL);

	geoclue_master_client_create_address_async(geo_master, geo_create_address, NULL);

	g_signal_connect(G_OBJECT(client), "invalidated", G_CALLBACK(geo_client_invalid), NULL);
	g_signal_connect(G_OBJECT(client), "address-provider-changed", G_CALLBACK(geo_address_change), NULL);

	return;
}

void
build_indicator (void)
{
	indicator = app_indicator_new_with_path("indicator-location", "indicator-location-unknown", APP_INDICATOR_CATEGORY_SYSTEM_SERVICES, ICON_DIR);
	app_indicator_set_title(indicator, _("Location"));
	app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

	GtkMenu * menu = GTK_MENU(gtk_menu_new());

	accuracy_item = GTK_MENU_ITEM(gtk_menu_item_new());
	gtk_widget_show(GTK_WIDGET(accuracy_item));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(accuracy_item));

	gtk_widget_show(GTK_WIDGET(menu));

	app_indicator_set_menu(indicator, menu);

	update_accuracy(GEOCLUE_ACCURACY_LEVEL_NONE);

	return;
}

int
main (int argc, char * argv[])
{
	gtk_init(&argc, &argv);

	/* Setup geoclue */
	GeoclueMaster * master = geoclue_master_get_default();
	geoclue_master_create_client_async(master, geo_create_client, NULL);

	build_indicator();

	/* Mainloop */
	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	/* clean up */
	g_main_loop_unref(mainloop);
	mainloop = NULL;

	geo_address_clean();
	geo_client_clean();

	return 0;
}