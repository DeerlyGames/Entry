#include "EntryPublic.h"
// Make sure you export C functions. Use your own macros if you like.

#include <iostream>

typedef struct Entry_State
{
	float value;
}Entry_State;

struct Entry_State state;


// Reload() is called once every time a library is reloaded (also the first time).
ENTRY_INTERFACE int Init(struct Entry_Entity_Container* entities)
{
//	_entities->Add()
//	Entry_Add(entities, "hello", Create_Entity, NULL);
//	entities->Add("dsdc", NULL, 0);
	std::cout << "Reloading library press q to quit." << std::endl;
	return 0;
}

// Update() is called in a loop once loaded.
// You are responsible for delaying the loop.
ENTRY_INTERFACE int Update(Entry_State* _state)
{
	if (_state != NULL)
		memcpy(&state, _state, sizeof(state));
	_state = &state;

	std::cout << "Update" << state.value << std::endl;
	return 0;
}