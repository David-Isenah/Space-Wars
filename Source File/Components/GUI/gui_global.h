#ifndef GUI_GLOBAL_H_INCLUDED
#define GUI_GLOBAL_H_INCLUDED

namespace GUI
{
    const float MIN_TRANSMISSION_VALUE = 0.1f;
    const float MIN_TRANSMISSION_SCALE = 0.0001f;

    enum InputType{OnClick, OnRelease};
    enum PhaseStage{DefaultPhase = 0, HighlighPhase, PressedPhase, StartPhase, InactivePhase, LockedPhase};
    enum Direction{ Horizontal = 0, Vertical};
}

#endif // GUI_GLOBAL_H_INCLUDED
