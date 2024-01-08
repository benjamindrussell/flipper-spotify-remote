#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/icon_i.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/button_panel.h>
#include <gui/modules/text_box.h>
#include <gui/modules/loading.h>
#include <input/input.h>

/* generated by fbt from .png files in images folder */
#include <spotify_remote_icons.h>

// Derek Jamison's uart helper
#include "uart_helper.h"

#define DEVICE_BAUDRATE 115200
#define LINE_DELIMITER '\n'
#define INCLUDE_LINE_DELIMITER false

// enum for scenes
typedef enum {
    SPOTIFY_REMOTE_BUTTON_PANEL_SCENE,
    SPOTIFY_REMOTE_TEXT_BOX_SCENE,
    SPOTIFY_REMOTE_LOADING_SCENE,
    SPOTIFY_REMOTE_SCENE_COUNT,
} SpotifyRemoteScene;

// enum for referencing module's views
typedef enum {
    SPOTIFY_REMOTE_BUTTON_PANEL_VIEW,
    SPOTIFY_REMOTE_TEXT_BOX_VIEW,
    SPOTIFY_REMOTE_LOADING_VIEW,
} SpotifyRemoteView;

// enum for button panel incicies
typedef enum {
    PREV,
    NEXT,
    PLAY,
    PAUSE,
    SHUFFLE,
    REPEAT,
} ButtonPanelIndex;

// create app object
typedef struct SpotifyRemoteApp {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    ButtonPanel* button_panel;
    TextBox* text_box;
    Loading* loading;
    UartHelper* uart_helper;
    FuriString* message;
} SpotifyRemoteApp;

typedef enum {
    SPOTIFY_REMOTE_ON_RECEIVE_EVENT,
} SpotifyRemoteCustomEvent;

// handles data received from UART
static void uart_process_line(FuriString* line, void* context) {
    SpotifyRemoteApp* app = context;
    furi_string_set(app->message, line);
    scene_manager_handle_custom_event(app->scene_manager, SPOTIFY_REMOTE_ON_RECEIVE_EVENT);
}

// called when selection is made in button panel
void button_panel_on_select_callback(void* context, uint32_t index) {
    SpotifyRemoteApp* app = context;
    switch(index) {
    case PREV:
        uart_helper_send(app->uart_helper, "1\n", 2);
        break;
    case NEXT:
        uart_helper_send(app->uart_helper, "2\n", 2);
        break;
    case PLAY:
        uart_helper_send(app->uart_helper, "3\n", 2);
        break;
    case PAUSE:
        uart_helper_send(app->uart_helper, "4\n", 2);
        break;
    case SHUFFLE:
        uart_helper_send(app->uart_helper, "5\n", 2);
        break;
    case REPEAT:
        uart_helper_send(app->uart_helper, "6\n", 2);
        break;
    default:
        break;
    }
}

// button panel callbacks
void spotify_remote_button_panel_scene_on_enter(void* context) {
    SpotifyRemoteApp* app = context;

    button_panel_reserve(app->button_panel, 2, 3);
    button_panel_add_item(
        app->button_panel,
        PREV,
        0,
        0,
        6,
        16,
        &I_prev_19x20,
        &I_prev_hover_19x20,
        button_panel_on_select_callback,
        context);
    button_panel_add_icon(app->button_panel, 6, 38, &I_prev_text_19x5);
    button_panel_add_item(
        app->button_panel,
        NEXT,
        1,
        0,
        39,
        16,
        &I_next_19x20,
        &I_next_hover_19x20,
        button_panel_on_select_callback,
        context);
    button_panel_add_icon(app->button_panel, 39, 38, &I_next_text_19x6);

    button_panel_add_item(
        app->button_panel,
        PLAY,
        0,
        1,
        6,
        50,
        &I_play_19x20,
        &I_play_hover_19x20,
        button_panel_on_select_callback,
        context);
    button_panel_add_icon(app->button_panel, 6, 72, &I_play_text_19x5);

    button_panel_add_item(
        app->button_panel,
        PAUSE,
        1,
        1,
        39,
        50,
        &I_pause_19x20,
        &I_pause_hover_19x20,
        button_panel_on_select_callback,
        context);
    button_panel_add_icon(app->button_panel, 39, 72, &I_pause_text_23x5);

    button_panel_add_item(
        app->button_panel,
        SHUFFLE,
        0,
        2,
        6,
        84,
        &I_shuffle_19x20,
        &I_shuffle_hover_19x20,
        button_panel_on_select_callback,
        context);
    button_panel_add_icon(app->button_panel, 5, 106, &I_shuffle_text_21x5);

    button_panel_add_item(
        app->button_panel,
        REPEAT,
        1,
        2,
        39,
        84,
        &I_repeat_19x20,
        &I_repeat_hover_19x20,
        button_panel_on_select_callback,
        context);
    button_panel_add_icon(app->button_panel, 40, 106, &I_repeat_text_18x5);

    view_dispatcher_switch_to_view(app->view_dispatcher, SPOTIFY_REMOTE_BUTTON_PANEL_VIEW);
}

bool spotify_remote_button_panel_scene_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void spotify_remote_button_panel_scene_on_exit(void* context) {
    SpotifyRemoteApp* app = context;
    button_panel_reset(app->button_panel);
}

// text box callbacks
void spotify_remote_text_box_scene_on_enter(void* context) {
    SpotifyRemoteApp* app = context;
    text_box_reset(app->text_box);
    text_box_set_text(app->text_box, furi_string_get_cstr(app->message));
    text_box_set_font(app->text_box, TextBoxFontText);
    view_dispatcher_switch_to_view(app->view_dispatcher, SPOTIFY_REMOTE_TEXT_BOX_VIEW);
}

bool spotify_remote_text_box_scene_on_event(void* context, SceneManagerEvent event) {
    SpotifyRemoteApp* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case SPOTIFY_REMOTE_ON_RECEIVE_EVENT:
            if(furi_string_get_cstr(app->message)[0] == 'O' &&
               furi_string_get_cstr(app->message)[1] == 'K') {
                scene_manager_next_scene(app->scene_manager, SPOTIFY_REMOTE_BUTTON_PANEL_SCENE);
                consumed = true;
            }
            break;
        }
    default:
        break;
    }

    return consumed;
}

void spotify_remote_text_box_scene_on_exit(void* context) {
    SpotifyRemoteApp* app = context;
    text_box_reset(app->text_box);
}

// loading callbacks
void spotify_remote_loading_scene_on_enter(void* context) {
    SpotifyRemoteApp* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, SPOTIFY_REMOTE_LOADING_VIEW);
}

bool spotify_remote_loading_scene_on_event(void* context, SceneManagerEvent event) {
    SpotifyRemoteApp* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case SPOTIFY_REMOTE_ON_RECEIVE_EVENT:
            if(furi_string_get_cstr(app->message)[0] == 'I' &&
               furi_string_get_cstr(app->message)[1] == 'P') {
                scene_manager_next_scene(app->scene_manager, SPOTIFY_REMOTE_TEXT_BOX_SCENE);
                consumed = true;
            }
            break;
        }
    default:
        break;
    }

    return consumed;
}

void spotify_remote_loading_scene_on_exit(void* context) {
    UNUSED(context);
}

// array of on_enter handlers
void (*const spotify_remote_scene_on_enter_handlers[])(void*) = {
    spotify_remote_button_panel_scene_on_enter,
    spotify_remote_text_box_scene_on_enter,
    spotify_remote_loading_scene_on_enter,
};

// array of on_event handlers
bool (*const spotify_remote_scene_on_event_handlers[])(void*, SceneManagerEvent) = {
    spotify_remote_button_panel_scene_on_event,
    spotify_remote_text_box_scene_on_event,
    spotify_remote_loading_scene_on_event,
};

// array of on_exit handlers
void (*const spotify_remote_scene_on_exit_handlers[])(void*) = {
    spotify_remote_button_panel_scene_on_exit,
    spotify_remote_text_box_scene_on_exit,
    spotify_remote_loading_scene_on_exit,
};

// create custom event callback
static bool spotify_remote_custom_callback(void* context, uint32_t custom_event) {
    furi_assert(context);
    SpotifyRemoteApp* app = context;
    // delegate custom event handling to scene manager
    return scene_manager_handle_custom_event(app->scene_manager, custom_event);
}

// create back event callback
bool spotify_remote_back_event_callback(void* context) {
    furi_assert(context);
    SpotifyRemoteApp* app = context;
    // delegate back event to scene manager
    return scene_manager_handle_back_event(app->scene_manager);
}

// attach all handlers to the scene manager
static const SceneManagerHandlers spotify_remote_scene_manager_handlers = {
    .on_enter_handlers = spotify_remote_scene_on_enter_handlers,
    .on_event_handlers = spotify_remote_scene_on_event_handlers,
    .on_exit_handlers = spotify_remote_scene_on_exit_handlers,
    .scene_num = SPOTIFY_REMOTE_SCENE_COUNT,
};

// function to allocate memory and initialize the fields in the App struct
static SpotifyRemoteApp* spotify_remote_app_alloc() {
    // initialize app, scene manager, and view dispatcher
    SpotifyRemoteApp* app = malloc(sizeof(SpotifyRemoteApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->scene_manager = scene_manager_alloc(&spotify_remote_scene_manager_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // enable view dispatcher queue to handle events
    view_dispatcher_enable_queue(app->view_dispatcher);

    // set callbacks and context for view dispacher
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, spotify_remote_custom_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, spotify_remote_back_event_callback);

    // create and add views for modules
    app->button_panel = button_panel_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SPOTIFY_REMOTE_BUTTON_PANEL_VIEW,
        button_panel_get_view(app->button_panel));
    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SPOTIFY_REMOTE_TEXT_BOX_VIEW, text_box_get_view(app->text_box));
    app->loading = loading_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SPOTIFY_REMOTE_LOADING_VIEW, loading_get_view(app->loading));

    // Initialize the UART helper.
    app->uart_helper = uart_helper_alloc();
    uart_helper_set_baud_rate(app->uart_helper, DEVICE_BAUDRATE);
    uart_helper_set_delimiter(app->uart_helper, LINE_DELIMITER, INCLUDE_LINE_DELIMITER);
    uart_helper_set_callback(app->uart_helper, uart_process_line, app);

    app->message = furi_string_alloc();

    return app;
}

// free all data
static void spotify_remote_app_free(SpotifyRemoteApp* app) {
    uart_helper_free(app->uart_helper);

    furi_assert(app);
    view_dispatcher_remove_view(app->view_dispatcher, SPOTIFY_REMOTE_BUTTON_PANEL_VIEW);
    view_dispatcher_remove_view(app->view_dispatcher, SPOTIFY_REMOTE_TEXT_BOX_VIEW);
    view_dispatcher_remove_view(app->view_dispatcher, SPOTIFY_REMOTE_LOADING_VIEW);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);
    button_panel_free(app->button_panel);
    text_box_free(app->text_box);
    loading_free(app->loading);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t spotify_remote_app(void* p) {
    UNUSED(p);

    SpotifyRemoteApp* app = spotify_remote_app_alloc();
    scene_manager_next_scene(app->scene_manager, SPOTIFY_REMOTE_LOADING_SCENE);
    view_dispatcher_run(app->view_dispatcher);

    spotify_remote_app_free(app);
    return 0;
}
