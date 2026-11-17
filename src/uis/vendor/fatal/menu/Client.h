#pragma once

#include <vector>
#include <algorithm>
#include <format>
#include "property/Properties.h"

namespace Client
{
	struct FeatureGroup
	{
		const char* name;
		float height;
		std::vector<Properties::PropDesc> features;
	};
	struct FeatureSubPage
	{
		const char* name;
		std::vector<FeatureGroup> groups;
	};
	struct FeaturePage
	{
		const char* name;
		std::vector<FeatureSubPage> subpages;
	};
	struct FeatureCategory
	{
		char icon[4];   // host note: pack used char8_t[4]; the trainer builds
		                // with /Zc:char8_t- so u8 literals are plain char.
		const char* name;
		std::vector<FeaturePage> pages;
	};

	// ------------------------------------------------------------------
	// Project Acceleracers Trainer: the pack's test-prop section is replaced
	// with trainer bindings. Everything above (the structs) is verbatim.
	// Targets resolve through ClientBind so this header stays independent of
	// the trainer's own headers; ui_fatal.cpp defines the resolvers.
	// ------------------------------------------------------------------
	namespace ClientBind
	{
		enum BoolId
		{
			B_UnlimitedNitrox, B_UnlimitedJumpJets, B_PerfectHandling,
			B_Noclip, B_GodMode, B_HydroWater, B_NoSandStall,
			B_AccelechargerBoost, B_EngineBoost,
			B_UnlockCars, B_UnlockAccel, B_SaveProgress, B_SetRecords, B_FinishRealm,
			B_Count
		};
		enum FloatId
		{
			F_Grip, F_EngineHp, F_NoclipSpeed, F_SandFloor, F_AccelechargerUses, F_RecordSeconds,
			F_Count
		};

		bool*  BoolTarget(int id);   // persistent toggles backed by g_cfg
		bool*  ActionTarget(int id); // one-shots: ui_fatal.cpp queues and clears
		float* FloatTarget(int id);  // sliders backed by g_cfg
	}

	class TrainerBool : public Properties::BoolProperty
	{
		bool* target;
	protected:
		bool processValue(bool valIn) override
		{
			*target = valIn;
			return valIn;
		}
	public:
		TrainerBool(bool* targetIn, bool defaultValue)
			: BoolProperty(defaultValue), target(targetIn)
		{}
	};

	class TrainerFloat : public Properties::NumberProp
	{
		float* target;
	protected:
		double processValue(double valIn) override
		{
			const double v = processEx(valIn, getMin(), getMax(), getStep());
			*target = static_cast<float>(v);
			return v;
		}
	public:
		TrainerFloat(double minIn, double maxIn, double stepIn,
		             float* targetIn, const char* formatIn = "{}")
			: NumberProp(minIn, maxIn, stepIn, static_cast<double>(*targetIn), formatIn),
			  target(targetIn)
		{}
	};

	// bound props
	inline TrainerBool nitroProp    { ClientBind::BoolTarget(ClientBind::B_UnlimitedNitrox),    false };
	inline TrainerBool jetsProp     { ClientBind::BoolTarget(ClientBind::B_UnlimitedJumpJets),  false };
	inline TrainerBool handlingProp { ClientBind::BoolTarget(ClientBind::B_PerfectHandling),    false };
	inline TrainerBool noclipProp   { ClientBind::BoolTarget(ClientBind::B_Noclip),             false };
	inline TrainerBool godProp      { ClientBind::BoolTarget(ClientBind::B_GodMode),            false };
	inline TrainerBool hydroProp    { ClientBind::BoolTarget(ClientBind::B_HydroWater),         false };
	inline TrainerBool sandProp     { ClientBind::BoolTarget(ClientBind::B_NoSandStall),        false };
	inline TrainerBool accelProp    { ClientBind::BoolTarget(ClientBind::B_AccelechargerBoost), false };
	inline TrainerBool engineProp   { ClientBind::BoolTarget(ClientBind::B_EngineBoost),        false };
	inline TrainerBool unlockCars   { ClientBind::ActionTarget(ClientBind::B_UnlockCars),       false };
	inline TrainerBool unlockAccel  { ClientBind::ActionTarget(ClientBind::B_UnlockAccel),      false };
	inline TrainerBool saveProgress { ClientBind::ActionTarget(ClientBind::B_SaveProgress),     false };
	inline TrainerBool setRecords   { ClientBind::ActionTarget(ClientBind::B_SetRecords),       false };
	inline TrainerBool finishRealm  { ClientBind::ActionTarget(ClientBind::B_FinishRealm),      false };

	inline TrainerFloat gripProp     { 2,    30,    0.1, ClientBind::FloatTarget(ClientBind::F_Grip),              "{:.1f}" };
	inline TrainerFloat engineHpProp { 100,  20000, 50,  ClientBind::FloatTarget(ClientBind::F_EngineHp),          "{:.0f} HP" };
	inline TrainerFloat noclipSpeed  { 300,  8000,  50,  ClientBind::FloatTarget(ClientBind::F_NoclipSpeed),       "{:.0f}" };
	inline TrainerFloat sandFloorProp{ 200,  3000,  50,  ClientBind::FloatTarget(ClientBind::F_SandFloor),         "{:.0f}" };
	inline TrainerFloat accelUses    { 0,    999,   1,   ClientBind::FloatTarget(ClientBind::F_AccelechargerUses), "{:.0f}" };
	inline TrainerFloat recordSecs   { 0,    10,    0.1, ClientBind::FloatTarget(ClientBind::F_RecordSeconds),     "{:.2f} s" };

	const std::vector<FeatureCategory> FeatureCategories = {
		FeatureCategory(
			u8"\ue66f", "VEHICLE",
			{
				FeaturePage(
					"Vehicle",
					{
						FeatureSubPage(
							"General",
							{
								FeatureGroup(
									"TUNING", -1,
									{
										Properties::PropDesc("Unlimited Nitrox", &nitroProp),
										Properties::PropDesc("Unlimited Jump Jets", &jetsProp),
										Properties::PropDesc("Perfect Handling", &handlingProp),
										Properties::PropDesc("Grip", &gripProp)
									}
								),
								FeatureGroup(
									"ACCELECHARGER", -1,
									{
										Properties::PropDesc("Accelecharger Charges", &accelProp),
										Properties::PropDesc("Charges", &accelUses)
									}
								),
								FeatureGroup(
									"ENGINE", -1,
									{
										Properties::PropDesc("Engine HP Boost", &engineProp),
										Properties::PropDesc("Engine HP", &engineHpProp)
									}
								),
								FeatureGroup(
									"HAZARDS", -1,
									{
										Properties::PropDesc("Hydro: No Water Slow", &hydroProp),
										Properties::PropDesc("Spike: No Sand Stall", &sandProp),
										Properties::PropDesc("Sand Floor", &sandFloorProp)
									}
								)
							}
						)
					}
				)
			}
		),
		FeatureCategory(
			u8"\ue6df", "PLAYER",
			{
				FeaturePage(
					"Player",
					{
						FeatureSubPage(
							"General",
							{
								FeatureGroup(
									"MOVEMENT", -1,
									{
										Properties::PropDesc("Noclip", &noclipProp),
										Properties::PropDesc("Noclip Speed", &noclipSpeed)
									}
								),
								FeatureGroup(
									"SURVIVAL", -1,
									{
										Properties::PropDesc("God Mode", &godProp)
									}
								)
							}
						)
					}
				)
			}
		),
		FeatureCategory(
			u8"\ue620", "ACTIONS",
			{
				FeaturePage(
					"Actions",
					{
						FeatureSubPage(
							"General",
							{
								FeatureGroup(
									"UNLOCKS", -1,
									{
										Properties::PropDesc("Unlock All Cars", &unlockCars),
										Properties::PropDesc("Unlock All AcceleChargers", &unlockAccel),
										Properties::PropDesc("Save Progress", &saveProgress)
									}
								),
								FeatureGroup(
									"RECORDS", -1,
									{
										Properties::PropDesc("Record Seconds", &recordSecs),
										Properties::PropDesc("Set All Realm Records", &setRecords),
										Properties::PropDesc("Complete Current Realm", &finishRealm)
									}
								)
							}
						)
					}
				)
			}
		),
		FeatureCategory(
			u8"\ue665", "WORLD",
			{
				FeaturePage(
					"General",
					{
						FeatureSubPage("General")
					}
				)
			}
		)
	};
}