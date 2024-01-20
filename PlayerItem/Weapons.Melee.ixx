export module Weapons:Melee;

export import PlayerItem;

// #TODO override the original CKnife class.
export class CKnife2 : public CPrefabWeapon
{
public:
	void Spawn() noexcept override;
	void Precache() noexcept override;
	qboolean GetItemInfo(ItemInfo* p) noexcept override;
	qboolean CanDrop() noexcept override { return false; }
	qboolean Deploy() noexcept override;
	void Holster(int skiplocal) noexcept override;
	float GetMaxSpeed() noexcept override { return m_fMaxSpeed; }
	int iItemSlot() noexcept override { return 3; }
	void PrimaryAttack() noexcept override;
	void SecondaryAttack() noexcept override;
	qboolean UseDecrement() noexcept override { return true; }
	void WeaponIdle() noexcept override;

public:
	inline void Stab() noexcept;
	inline void Swing() noexcept;

public:
	bool ShieldSecondaryFire(int iUpAnim, int iDownAnim) noexcept;
	void SetPlayerShieldAnim() noexcept;
	void ResetPlayerShieldAnim() noexcept;

private:
	static inline unsigned short m_usKnife{};

public:

	static inline constexpr auto KNIFE_BODYHIT_VOLUME = 128;
	static inline constexpr auto KNIFE_WALLHIT_VOLUME = 512;
	static inline constexpr float KNIFE_MAX_SPEED = 250.0f;
	static inline constexpr float KNIFE_MAX_SPEED_SHIELD = 180.0f;
	static inline constexpr float KNIFE_STAB_DAMAGE = 65.0f;
	static inline constexpr float KNIFE_SWING_DAMAGE = 15.0f;
	static inline constexpr float KNIFE_SWING_DAMAGE_FAST = 20.0f;
	static inline constexpr float KNIFE_STAB_DISTANCE = 32.0f;
	static inline constexpr float KNIFE_SWING_DISTANCE = 48.0f;

	static inline constexpr char CLASSNAME[] = "weapon_knife";
};
