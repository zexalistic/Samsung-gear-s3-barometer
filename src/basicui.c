#include "basicui.h"
#include <sensor.h>
#include <storage.h>
#include <runtime_info.h>

#define LISTENER_TIMEOUT 0
#define LISTENER_TIMEOUT_FINAL ((LISTENER_TIMEOUT != 0) ? LISTENER_TIMEOUT : 100)
#define SENSOR_COUNT 14

int count=0;
char buff[400];
char *cur = buff;

char *filepath="/home/owner/share/four.txt";

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *button;
	float pressure;
} appdata_s;

void write_file(const char* filepath, const char* buf)
{
    FILE *fp;
    fp = fopen(filepath, "a");
    fputs(buf, fp);
    fclose(fp);
}

static void barometer_cb(sensor_h sensor, sensor_event_s *event, void *data)
{
	char tmp[10];

	char *str = "\n";


	appdata_s * ad = (appdata_s *)data;

	ad->pressure = (float)event->values[0];
	gcvt(ad->pressure,9,tmp);
	strcat(tmp,str);
	write_file(filepath, tmp);

	//elm_object_text_set(ad->button, tmp);

}

static void click_cb(void *data, Evas_Object *obj, void *event_info){
	char* str="cut\n";
	write_file(filepath, str);
}

static int
register_barometer_callback(appdata_s *ad)
{
	int error;
	bool supported;
	sensor_h barometer;
	sensor_listener_h barometerListener;

	error = sensor_is_supported( SENSOR_PRESSURE, &supported );
	if(error != SENSOR_ERROR_NONE && supported){
		return error;
	}

	error = sensor_get_default_sensor(SENSOR_PRESSURE, &barometer);
	if(error != SENSOR_ERROR_NONE){
		return error;
	}

	error = sensor_create_listener( barometer, &barometerListener);
	if(error != SENSOR_ERROR_NONE){
		return error;
	}

	sensor_listener_set_interval(barometerListener,10);
	error = sensor_listener_set_event_cb( barometerListener, 10, barometer_cb, ad );
	if(error != SENSOR_ERROR_NONE){
		return error;
	}

	sensor_listener_set_option(barometerListener, SENSOR_OPTION_ALWAYS_ON);

	error = sensor_listener_start( barometerListener );

	return SENSOR_ERROR_NONE;
}


static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}




static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	/* Create and initialize elm_win.
	   elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	   elm_conformant is mandatory for base gui to have proper size
	   when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Create an actual view of the base gui.
	   Modify this part to change the view. */
	ad->button = elm_button_add(ad->conform);
	elm_object_text_set(ad->button, "</align=center>Hello world</align>");
	evas_object_smart_callback_add(ad->button, "clicked", click_cb, ad);
	evas_object_size_hint_weight_set(ad->button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(ad->conform, ad->button);

	ad->pressure = 0;

	register_barometer_callback(ad);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool
app_create(void *data)
{
	appdata_s *ad = data;

	create_base_gui(ad);


	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;
	time(NULL);

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
