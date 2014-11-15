
#include <glm/glm.hpp>

#include "systems/System.h"
#include "base/Frequency.h"

struct WeaponComponent {
    WeaponComponent(): fireSpeed(0), reloadSpeed(0), bulletPerShot(1), bulletSpeed(5), precision(0), fire(false), reload(false), ammoLeftInClip(0) {}

    Frequency<float> fireSpeed;
    Frequency<float> reloadSpeed;
    int bulletPerShot;
    float bulletSpeed;

    float precision;

    bool fire, reload;
    int ammoLeftInClip;
};

#define theWeaponSystem WeaponSystem::GetInstance()
#if SAC_DEBUG
#define WEAPON(e) theWeaponSystem.Get(e,true,__FILE__,__LINE__)
#else
#define WEAPON(e) theWeaponSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Weapon)
};
