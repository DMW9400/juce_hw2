/*
  ==============================================================================

    gui_record_play.h
    Created: 11 Oct 2024 7:25:49pm
    Author:  David Matthew Welch

  ==============================================================================
*/

#pragma once

enum AppState {
    IDLE,
    PLAYING,
    RECORDING
};

extern AppState currentAppState;

