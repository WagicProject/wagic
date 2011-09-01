#include "PrecompiledHeader.h"

#include "Subtypes.h"

Subtypes * Subtypes::subtypesList = NEW Subtypes();

Subtypes::Subtypes()
{
    //Add the more common types, so that they can be accessed through ints
    //these should be added in the same order as the enum defined in subtypes.h!!!
    find("Creature");
    find("Enchantment");
    find("Sorcery");
    find("Instant");
    find("Land");
    find("Artifact");
    find("Legendary");
    find("Snow");
    find("Basic");
    find("World");
    find("Equipment");
    find("Aura");
    find("Planeswalker");
    find("Tribal");
    find("Plane");
    find("Scheme");
    find("Vanguard");
}

int Subtypes::find(string value, bool forceAdd)
{
    if (!value.size())
        return 0;

    if (value[0] >= 97 && value[0] <= 122) value[0] -= 32; //Poor man's camelcase. We assume strings we get are either Camelcased or lowercase
    map<string, int>::iterator it = values.find(value);
    if (it != values.end()) return it->second;
    if (!forceAdd) return 0;
    int id = (int) (valuesById.size() + 1);
    values[value] = id;
    valuesById.push_back(value);
    return id;
}

// Adds a subtype to the list, and associated it with a parent type.
//The association can happen only once, a subtype is then definitely associated to its parent type.
// If you associate "goblin" to "creature", trying to associate "goblin" to "land" afterwards will fail. "goblin" will stay associated to its first parent.
int Subtypes::add(string value, unsigned int parentType)
{
    unsigned int subtype = find(value);
    if (parentType && isSubType(subtype))
    {
        if ((unsigned int)(subtypesToType.size()) < subtype + 1)
            subtypesToType.resize(1 + subtype * 2, 0); //multiplying by 2 to avoid resizing at every insertion
        subtypesToType[subtype] = parentType;
    }
    return subtype;
}

string Subtypes::find(unsigned int id)
{
    if (valuesById.size() < id || !id) return "";
    return valuesById[id - 1];
}

bool  Subtypes::isSubtypeOfType(unsigned int subtype, unsigned int type)
{
    if(subtype >= size_t(subtypesToType.size()))
        return false;
    return (subtypesToType[subtype] == type);
}

bool Subtypes::isSuperType(unsigned int type)
{
    return (type == TYPE_BASIC || type == TYPE_WORLD || type == TYPE_SNOW || type == TYPE_LEGENDARY);
}

bool Subtypes::isType(unsigned int type)
{
    return (
        type == TYPE_CREATURE ||
        type == TYPE_ENCHANTMENT ||
        type == TYPE_SORCERY  ||
        type == TYPE_INSTANT ||
        type == TYPE_LAND  ||
        type == TYPE_ARTIFACT ||
        type == TYPE_PLANESWALKER  ||
        type == TYPE_TRIBAL ||
        type == TYPE_PLANE ||
        type == TYPE_SCHEME ||
        type == TYPE_VANGUARD 
    );
}

bool Subtypes::isSubType(unsigned int type)
{
    return (!isSuperType(type) && !isType(type));
}

const vector<string>& Subtypes::getValuesById()
{
    return valuesById;
}