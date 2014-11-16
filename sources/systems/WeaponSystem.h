
#include <glm/glm.hpp>

#include "systems/System.h"
#include "base/Frequency.h"

struct WeaponComponent {
    WeaponComponent(): fireSpeed(0), reloadSpeed(0), bulletPerShot(1), bulletSpeed(5), heatupPerBullet(0), coolingSpeed(0), precision(0), fire(false), reload(false), ammoLeftInClip(0), _hot(0), _mustCoolDown(false) {}

    Frequency<float> fireSpeed;
    Frequency<float> reloadSpeed;
    int bulletPerShot;
    float bulletSpeed;

    float heatupPerBullet;
    float coolingSpeed;

    float precision;

    bool fire, reload;
    int ammoLeftInClip;

    float _hot;
    bool _mustCoolDown;
};

#define theWeaponSystem WeaponSystem::GetInstance()
#if SAC_DEBUG
#define WEAPON(e) theWeaponSystem.Get(e,true,__FILE__,__LINE__)
#else
#define WEAPON(e) theWeaponSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Weapon)
};
