#include QMK_KEYBOARD_H
#include "timer.h"

enum layer_names {
    _QWERTY,
    _LOWER,
    _RAISE,
    _COLEMAK_DH,
    _VIMNAV,
#ifdef MOUSEKEY_ENABLE
    _MOUSE,
#endif
   _ADJUST,
};

#define RAISE MO(_RAISE)
#define LOWER MO(_LOWER)
#define VIMNAV MO(_VIMNAV)

// Left-hand home row mods
#define HOME_A LALT_T(KC_A)
#define HOME_S LGUI_T(KC_S)
#define HOME_D LSFT_T(KC_D)
#define HOME_F LCTL_T(KC_F)

// Right-hand home row mods
#define HOME_J RCTL_T(KC_J)
#define HOME_K RSFT_T(KC_K)
#define HOME_L RGUI_T(KC_L)
#define HOME_SCLN RALT_T(KC_SCLN)


#define HRM_MIN_HOLD 250 // milliseconds
static uint16_t hrm_timer[256];
static uint8_t left_mod_count  = 0;
static uint8_t right_mod_count = 0;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
//    ┌──────┬────────┬────────┬────────┬────────┬────────┐               ┌───────┬────────┬────────┬────────┬───────────┬──────┐
//    │ esc  │   1    │   2    │   3    │   4    │   5    │               │   6   │   7    │   8    │   9    │     0     │ bspc │
//    ├──────┼────────┼────────┼────────┼────────┼────────┤               ├───────┼────────┼────────┼────────┼───────────┼──────┤
//    │ tab  │   q    │   w    │   e    │   r    │   t    │               │   y   │   u    │   i    │   o    │     p     │  -   │
//    ├──────┼────────┼────────┼────────┼────────┼────────┤               ├───────┼────────┼────────┼────────┼───────────┼──────┤
//    │ lsft │ HOME_A │ HOME_S │ HOME_D │ HOME_F │   g    │               │   h   │ HOME_J │ HOME_K │ HOME_L │ HOME_SCLN │  '   │
//    ├──────┼────────┼────────┼────────┼────────┼────────┤               ├───────┼────────┼────────┼────────┼───────────┼──────┤
//    │ lctl │   z    │   x    │   c    │   v    │   b    │               │   n   │   m    │   ,    │   .    │     /     │  \   │
//    └──────┴────────┼────────┼────────┼────────┴────────┘               └───────┴────────┼────────┼────────┼───────────┴──────┘
//                    │   [    │   ]    │                                                  │   +    │   =    │
//                    └────────┴────────┼────────┬────────┐               ┌───────┬────────┼────────┴────────┘
//                                      │  lgui  │  spc   │               │ rgui  │  ent   │
//                                      └────────┼────────┼─────┐   ┌─────┼───────┼────────┘
//                                               │ VIMNAV │ tab │   │ del │ RAISE │
//                                               ├────────┼─────┤   ├─────┼───────┤
//                                               │  bspc  │  `  │   │ end │ lalt  │
//                                               └────────┴─────┘   └─────┴───────┘
[_QWERTY] = LAYOUT_5x6(
  KC_ESC  , KC_1   , KC_2    , KC_3    , KC_4    , KC_5    ,                       KC_6    , KC_7   , KC_8    , KC_9   , KC_0      , KC_BSPC,
  KC_TAB  , KC_Q   , KC_W    , KC_E    , KC_R    , KC_T    ,                       KC_Y    , KC_U   , KC_I    , KC_O   , KC_P      , KC_MINS,
  KC_LSFT , HOME_A , HOME_S  , HOME_D  , HOME_F  , KC_G    ,                       KC_H    , HOME_J , HOME_K  , HOME_L , HOME_SCLN , KC_QUOT,
  KC_LCTL , KC_Z   , KC_X    , KC_C    , KC_V    , KC_B    ,                       KC_N    , KC_M   , KC_COMM , KC_DOT , KC_SLSH   , KC_BSLS,
                     KC_LBRC , KC_RBRC ,                                                              KC_PLUS , KC_EQL                      ,
                                         KC_LGUI , KC_SPC  ,                       KC_RGUI , KC_ENT                                         ,
                                                   VIMNAV  , KC_TAB ,     KC_DEL , RAISE                                                    ,
                                                   KC_BSPC , KC_GRV ,     KC_END , KC_LALT
),

//    ┌─────┬──────┬──────┬──────┬─────┬─────┐               ┌─────┬──────┬──────┬──────┬─────┬─────┐
//    │  ~  │  !   │  @   │  #   │  $  │  %  │               │  ^  │  &   │  *   │  (   │  )  │ del │
//    ├─────┼──────┼──────┼──────┼─────┼─────┤               ├─────┼──────┼──────┼──────┼─────┼─────┤
//    │     │      │      │      │     │  [  │               │  ]  │ kp_7 │ kp_8 │ kp_9 │     │  +  │
//    ├─────┼──────┼──────┼──────┼─────┼─────┤               ├─────┼──────┼──────┼──────┼─────┼─────┤
//    │     │ home │ pgup │ pgdn │ end │  (  │               │  )  │ kp_4 │ kp_5 │ kp_6 │  -  │  |  │
//    ├─────┼──────┼──────┼──────┼─────┼─────┤               ├─────┼──────┼──────┼──────┼─────┼─────┤
//    │     │      │      │      │     │     │               │     │ kp_1 │ kp_2 │ kp_3 │  =  │  _  │
//    └─────┴──────┼──────┼──────┼─────┴─────┘               └─────┴──────┼──────┼──────┼─────┴─────┘
//                 │      │ pscr │                                        │ kp_0 │      │
//                 └──────┴──────┼─────┬─────┐               ┌─────┬──────┼──────┴──────┘
//                               │     │     │               │     │      │
//                               └─────┼─────┼─────┐   ┌─────┼─────┼──────┘
//                                     │     │     │   │     │     │
//                                     ├─────┼─────┤   ├─────┼─────┤
//                                     │     │     │   │     │     │
//                                     └─────┴─────┘   └─────┴─────┘
[_LOWER] = LAYOUT_5x6(
  KC_TILD , KC_EXLM , KC_AT   , KC_HASH , KC_DLR  , KC_PERC ,                         KC_CIRC , KC_AMPR , KC_ASTR , KC_LPRN , KC_RPRN , KC_DEL ,
  _______ , _______ , _______ , _______ , _______ , KC_LBRC ,                         KC_RBRC , KC_P7   , KC_P8   , KC_P9   , _______ , KC_PLUS,
  _______ , KC_HOME , KC_PGUP , KC_PGDN , KC_END  , KC_LPRN ,                         KC_RPRN , KC_P4   , KC_P5   , KC_P6   , KC_MINS , KC_PIPE,
  _______ , _______ , _______ , _______ , _______ , _______ ,                         _______ , KC_P1   , KC_P2   , KC_P3   , KC_EQL  , KC_UNDS,
                      _______ , KC_PSCR ,                                                                 KC_P0   , _______                    ,
                                          _______ , _______ ,                         _______ , _______                                        ,
                                                    _______ , _______ ,     _______ , _______                                                  ,
                                                    _______ , _______ ,     _______ , _______
),

//    ┌─────┬──────┬─────┬──────┬──────┬─────┐               ┌─────┬──────┬──────┬──────┬──────┬──────┐
//    │ f12 │  f1  │ f2  │  f3  │  f4  │ f5  │               │ f6  │  f7  │  f8  │  f9  │ f10  │ f11  │
//    ├─────┼──────┼─────┼──────┼──────┼─────┤               ├─────┼──────┼──────┼──────┼──────┼──────┤
//    │     │      │     │      │      │  [  │               │  ]  │      │ nUM  │ ins  │ sCRL │ mute │
//    ├─────┼──────┼─────┼──────┼──────┼─────┤               ├─────┼──────┼──────┼──────┼──────┼──────┤
//    │     │ left │ up  │ down │ rght │  (  │               │  )  │ mprv │ mply │ mnxt │      │ volu │
//    ├─────┼──────┼─────┼──────┼──────┼─────┤               ├─────┼──────┼──────┼──────┼──────┼──────┤
//    │     │      │     │      │      │     │               │     │      │      │      │      │ vold │
//    └─────┴──────┼─────┼──────┼──────┴─────┘               └─────┴──────┼──────┼──────┼──────┴──────┘
//                 │     │      │                                         │  =   │      │
//                 └─────┴──────┼──────┬─────┐               ┌─────┬──────┼──────┴──────┘
//                              │      │     │               │     │      │
//                              └──────┼─────┼─────┐   ┌─────┼─────┼──────┘
//                                     │     │     │   │     │     │
//                                     ├─────┼─────┤   ├─────┼─────┤
//                                     │     │     │   │     │     │
//                                     └─────┴─────┘   └─────┴─────┘
[_RAISE] = LAYOUT_5x6(
  KC_F12  , KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5   ,                         KC_F6   , KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_F11 ,
  _______ , _______ , _______ , _______ , _______ , KC_LBRC ,                         KC_RBRC , _______ , KC_NUM  , KC_INS  , KC_SCRL , KC_MUTE,
  _______ , KC_LEFT , KC_UP   , KC_DOWN , KC_RGHT , KC_LPRN ,                         KC_RPRN , KC_MPRV , KC_MPLY , KC_MNXT , _______ , KC_VOLU,
  _______ , _______ , _______ , _______ , _______ , _______ ,                         _______ , _______ , _______ , _______ , _______ , KC_VOLD,
                      _______ , _______ ,                                                                 KC_EQL  , _______                    ,
                                          _______ , _______ ,                         _______ , _______                                        ,
                                                    _______ , _______ ,     _______ , _______                                                  ,
                                                    _______ , _______ ,     _______ , _______
),

//    ┌─────┬─────┬─────┬─────┬─────┬─────┐               ┌──────┬──────┬─────┬──────┬──────┬──────┐
//    │     │     │     │     │     │     │               │  f6  │  f7  │ f8  │  f9  │ f10  │ f11  │
//    ├─────┼─────┼─────┼─────┼─────┼─────┤               ├──────┼──────┼─────┼──────┼──────┼──────┤
//    │     │     │     │     │     │     │               │      │      │     │      │ sCRL │ mute │
//    ├─────┼─────┼─────┼─────┼─────┼─────┤               ├──────┼──────┼─────┼──────┼──────┼──────┤
//    │     │     │     │     │     │     │               │ left │ down │ up  │ rght │      │ volu │
//    ├─────┼─────┼─────┼─────┼─────┼─────┤               ├──────┼──────┼─────┼──────┼──────┼──────┤
//    │     │     │     │     │     │     │               │      │      │     │      │      │ vold │
//    └─────┴─────┼─────┼─────┼─────┴─────┘               └──────┴──────┼─────┼──────┼──────┴──────┘
//                │     │     │                                         │     │      │
//                └─────┴─────┼─────┬─────┐               ┌──────┬──────┼─────┴──────┘
//                            │     │     │               │      │      │
//                            └─────┼─────┼─────┐   ┌─────┼──────┼──────┘
//                                  │     │     │   │     │      │
//                                  ├─────┼─────┤   ├─────┼──────┤
//                                  │     │     │   │     │      │
//                                  └─────┴─────┘   └─────┴──────┘
[_VIMNAV] = LAYOUT_5x6(
  _______ , _______ , _______ , _______ , _______ , _______ ,                         KC_F6   , KC_F7   , KC_F8   , KC_F9    , KC_F10  , KC_F11 ,
  _______ , _______ , _______ , _______ , _______ , _______ ,                         _______ , _______ , _______ , _______  , KC_SCRL , KC_MUTE,
  _______ , _______ , _______ , _______ , _______ , _______ ,                         KC_LEFT , KC_DOWN , KC_UP   , KC_RIGHT , _______ , KC_VOLU,
  _______ , _______ , _______ , _______ , _______ , _______ ,                         _______ , _______ , _______ , _______  , _______ , KC_VOLD,
                      _______ , _______ ,                                                                 _______ , _______                     ,
                                          _______ , _______ ,                         _______ , _______                                         ,
                                                    _______ , _______ ,     _______ , _______                                                   ,
                                                    _______ , _______ ,     _______ , _______
)
};

bool is_home_mod(uint16_t keycode) {
    switch (keycode) {
        case HOME_A:
        case HOME_S:
        case HOME_D:
        case HOME_F:

        case HOME_J:
        case HOME_K:
        case HOME_L:
        case HOME_SCLN:

            return true;
    }
    return false;
}

bool is_left_hand(keyrecord_t *record) {
    return record->event.key.col < MATRIX_COLS / 2;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (!is_home_mod(keycode)) return true;
    bool left = is_left_hand(record);
    if (record->event.pressed) {
        hrm_timer[keycode] = timer_read();
        if (left) left_mod_count++;
        else right_mod_count++;

        // Force tap if 2+ HRMs same side
        if ((left && left_mod_count >= 2) || (!left && right_mod_count >= 2)) {
            register_code16(keycode & 0xFF);
            unregister_code16(keycode & 0xFF);
            return false; // skip mod
        }

    } else {
        uint16_t held = timer_elapsed(hrm_timer[keycode]);

        if ((left && left_mod_count >= 2) || (!left && right_mod_count >= 2)) {
            if (left) left_mod_count--;
            else right_mod_count--;
            return false;
        }

        if (held < HRM_MIN_HOLD) {
            // Short tap → send keycode immediately
            register_code16(keycode & 0xFF);
            unregister_code16(keycode & 0xFF);
            if (left) left_mod_count--;
            else right_mod_count--;
            return false;
        }

        // Long hold → normal mod handled by MT()
        if (left) left_mod_count--;
        else right_mod_count--;
    }
    return true;
}

bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    if (!is_home_mod(keycode)) return true;
    bool left = is_left_hand(record);
    if ((left && left_mod_count >= 2) || (!left && right_mod_count >= 2)) {
        // Disable mod -> only send tap
        return false;
    }
    return false;
}

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        // Left hand home-row mods
        case HOME_A:
        case HOME_S:
        case HOME_D:
        case HOME_F:
            return 170;
        // Right hand home-row mods
        case HOME_J:
        case HOME_K:
        case HOME_L:
        case HOME_SCLN:
            return 170;
        default:
            return TAPPING_TERM;
    }
}

#ifdef OLED_ENABLE
#include <oled.c>
#endif
