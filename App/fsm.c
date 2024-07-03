#include "fsm.h"

uint32_t is_event_match(struct FSM_EVENT *event, struct FSM_ACTION *action)
{
    uint32_t ret = 0;

    if (event == NULL || action == NULL)
        return ret;

    if (event->event_id == action->event_id)
        ret = 1;

    return ret;
}

void fsm_handle_event(struct FSM_ENTITY *fsm, struct FSM_EVENT *event)
{
    struct FSM_STATE *current_state, *next_state;
    struct FSM_ACTION *current_action;
    int i;

    if ((fsm == NULL) || (fsm->name == NULL) || (fsm->state = NULL))
        return;

    current_state = fsm->state;
    if ((current_state->name == NULL) ||
        (current_state->action_list_count == 0) ||
        (current_state->action_list == NULL))
        return;
    
    for (i = 0; i < current_state->action_list_count; i++) {
        current_action = &current_state->action_list[i];
        if (is_event_match(event, current_action)) {
            if (current_action->guard_func)
                if (!current_action->guard_func(fsm, event))
                    continue;

            if (current_action->action_func)
                current_action->action_func(fsm, event);

            next_state = current_action->next_state;
            if ((next_state != NULL) && (next_state != current_state)) {
                if (current_state->leave_func)
                    current_state->leave_func(fsm, next_state);

                fsm->state = next_state;
                
                if (next_state->enter_func)
                    next_state->enter_func(fsm, current_state);
            }
            break;
        }
    } 
}