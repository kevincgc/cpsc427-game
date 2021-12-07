#include "tinyECS/tiny_ecs.hpp"
#include <string>
#include <iostream>

///////////////////////////
// OOP inheritance pattern
struct AnimalOOP {
	virtual bool canDive() = 0;
	virtual bool canWalk() = 0;
	virtual std::string name() = 0;
};

struct WaterAnimalOOP : public AnimalOOP {
	float swim_speed = 2;
	bool canDive() { return true; };
	bool canWalk() { return false; };
};

struct LandAnimalOOP : public AnimalOOP {
	float walk_speed = 3;
	bool canDive() { return false; };
	bool canWalk() { return true; };
};

struct FishOOP : public WaterAnimalOOP
{
	virtual std::string name() { return "Fish"; };
};

struct HorseOOP : public LandAnimalOOP
{
	virtual std::string name() { return "Horse"; };
};

struct TurtleOOP : public LandAnimalOOP, WaterAnimalOOP
{
	virtual std::string name() { return "Turtle"; };
};

/////////////////////////////////////////
// Entity Component System (ECS) pattern
struct Name {
	std::string name;
	Name(const char* str) : name(str) {};
};

struct Swims {
	float swim_speed = 3;
};

struct Walks {
	float walk_speed = 2;
};

struct Flies {
	float fly_speed = 8;
};

// For SoA
struct PositionVelocity {
	float position[2];
	float velocity[2];
	PositionVelocity(float x_pos, float y_pos, float x_vel, float y_vel) : 
		position { x_pos, y_pos	}, velocity{ x_vel, y_vel } {};
};

// For AoS
struct Value2D {
	float x;
	float y;
	Value2D(float x, float y) : x(x), y(y) {};
};

// Setup ECS
class RegistryECS
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<Name> names;
	ComponentContainer<Swims> swims;
	ComponentContainer<Walks> walks;
	ComponentContainer<Flies> flies;
	ComponentContainer<PositionVelocity> position_velocity_soa;
	ComponentContainer<Value2D*> position_velocity_aos;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	RegistryECS()
	{
		registry_list.push_back(&names);
		registry_list.push_back(&swims);
		registry_list.push_back(&walks);
		registry_list.push_back(&flies);
		registry_list.push_back(&position_velocity_soa);
		registry_list.push_back(&position_velocity_aos);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all regestry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

RegistryECS registry;

/////////////////////////////////////////
// Entry point
int main(int argc, char* argv[])
{
	/////////////////////////
	// OOP pattern
	// Create a fish and horse
	FishOOP fish_oop;
	HorseOOP horse_oop;

	// Create a turtle
	TurtleOOP turtle_oop;

	// Group all animals (enabled by inheriting the common base class Animal)
	std::vector<AnimalOOP*> animals_oop;
	animals_oop.push_back(&fish_oop);
	animals_oop.push_back(&horse_oop);
	//animals_oop.push_back(&turtle_oop); // ERROR: the base class is ambigious, see mroe here: https://stackoverflow.com/questions/44878627/inheritance-causes-ambiguous-conversion
	animals_oop.push_back(static_cast<LandAnimalOOP*>(&turtle_oop)); // WARNING: this compiles, but now the turtle is not able to swim!
	
	// Print the names and abilities of all animals
	std::cout << "----- OOP inheritance debug output -----\n";
	for (AnimalOOP* animalPtr : animals_oop) {
        assert(animalPtr);
        auto& theAnimal = *animalPtr; // NOTE: reference is needed for virtual function calls
        std::cout
            << theAnimal.name() << ' '
            << (theAnimal.canDive() ? "can" : "can't") << " swim and "
            << (theAnimal.canWalk() ? "can" : "can't") << " walk" << std::endl;
		// This is not what we want, our OOP turtle can't swim :/
    }

	//////////////////////////
	// ECS pattern
	// Create a fish
	Entity fish;
	registry.names.insert(fish, Name("Fish"));
	registry.swims.insert(fish, Swims());
	// Fish set at position (1,6) with velocity (2,2)
	registry.position_velocity_aos.insert(fish, (Value2D*) std::begin({ Value2D(1, 6), Value2D(2, 2) }));

	// Create a horse
	Entity horse;
	registry.names.emplace(horse, "Horse"); // Note, emplace() does the same as insert() but is shorter
	registry.walks.emplace(horse);

	// Create a turtle
	Entity turtle;
	registry.names.emplace(turtle, "Turtle");
	registry.walks.emplace(turtle);
	registry.swims.emplace(turtle);
	// Turtle set at position (4,2) with speed (0,3)
	registry.position_velocity_soa.insert(turtle, PositionVelocity(4, 2, 0, 3));

	// Create American Dipper
	Entity american_dipper;
	registry.names.emplace(american_dipper, "American Dipper");
	registry.walks.emplace(american_dipper);
	registry.swims.emplace(american_dipper);
	registry.flies.emplace(american_dipper);

	// Remove the horse
	registry.remove_all_components_of(horse);

	// Fish aged during program execution
	Name *fish_name = &registry.names.get(fish);
	fish_name->name = "Old " + fish_name->name;

	// Note, no need to group animals, the tinyECS registry has all the components in a list automatically!
	// Note, no need to define fish, horse, and turtle classed, they are formed by the equipped components!

	// Print the names and abilities of all the animals
	std::cout << "----- ECS debug output -----\n";
	for (Entity& animal : registry.names.entities) {
        std::cout
            << registry.names.get(animal).name << ' '
            << (registry.swims.has(animal) ? "can" : "can't") << " swim, "
            << (registry.walks.has(animal) ? "can" : "can't") << " walk, and "
			<< (registry.flies.has(animal) ? "can" : "can't") << " fly " << std::endl;
    }

	// Print position/velocity of fish and turtle
	std::cout << registry.names.get(fish).name << " is at (" << registry.position_velocity_aos.get(fish)[0].x
		<< ',' << registry.position_velocity_aos.get(fish)[0].y << ") with velocity ("
		<< registry.position_velocity_aos.get(fish)[1].x << ',' << registry.position_velocity_aos.get(fish)[1].y
		<< ") using AoS" << std::endl;
	std::cout << registry.names.get(turtle).name << " is at (" << registry.position_velocity_soa.get(turtle).position[0]
		<< ',' << registry.position_velocity_soa.get(turtle).position[1] << ") with velocity ("
		<< registry.position_velocity_soa.get(turtle).velocity[0] << ',' << registry.position_velocity_soa.get(turtle).velocity[1]
		<< ") using SoA" << std::endl;

	// Inspect the ECS state
	registry.list_all_components();
	registry.list_all_components_of(turtle);

	// Clearing the ECS system before exit
	registry.clear_all_components();
	return EXIT_SUCCESS;
}

