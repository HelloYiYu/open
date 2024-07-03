#ifndef __FSM_H__
#define __FSM_H__

#include "cmsis_os.h"
#include "my_list.h"

struct FSM_STATE;
struct FSM_ENTITY;
struct FSM_EVENT;

typedef void (*FSM_ENTER_FUNC)(struct FSM_ENTITY *, struct FSM_STATE *);
typedef void (*FSM_LEAVE_FUNC)(struct FSM_ENTITY *, struct FSM_STATE *);
typedef uint32_t (*FSM_GUARD_FUNC)(struct FSM_ENTITY *, struct FSM_EVENT *);
typedef void (*FSM_ACTION_FUNC)(struct FSM_ENTITY *, struct FSM_EVENT *);

struct FSM_EVENT {
    struct list_head node;

    uint32_t event_id;
    uint32_t *name;
    
    void *param;
};

struct FSM_ACTION {
    uint32_t event_id;
    FSM_GUARD_FUNC guard_func;
    FSM_ACTION_FUNC action_func;
    struct FSM_STATE *next_state;
};

struct FSM_STATE {
    char *name;
    FSM_ENTER_FUNC enter_func;
    FSM_LEAVE_FUNC leave_func;
    uint32_t action_list_count;
    struct FSM_ACTION *action_list;
};

struct FSM_ENTITY {
    char *name;
    
    struct FSM_STATE *state;
};


void fsm_handle_event(struct FSM_ENTITY *fsm, struct FSM_EVENT *event);

#endif
