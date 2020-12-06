#include "../include/Astroid.h"
#include "../include/DestroyAstroid.h"
#include "../include/HitAstroid.h"
#include "../include/SplitAstroid.h"
#include "../include/Levels.h"
#include "../include/Player.h"
#include "../../AAEngine/include/Lights.h"
#include "../../AAEngine/include/Loop.h"
#include "../../AAEngine/include/Sound/SoundDevice.h"
#include "../../AAEngine/include/Sound/LongSound.h"
#include "../../AAEngine/include/Sound/ShortSound.h"
#include "../../AAEngine/include/Sound/SoundListener.h"
#include "../../AAEngine/include/CollisionHandler.h"
#include <glm/glm.hpp>

using namespace AA;

int main()
{
	// set some basic defaults for our astroid game
	static int cam_1 = LOOP->addCamera();
	LOOP->getCamera(cam_1).shiftYawAndPitch(0.f, -90.f); // look down
	DISPLAY->setWindowTitle("ASTROIDS!");
	DISPLAY->setWindowSize(800, 800, true);

	// INIT SOUND
	AA::SoundDevice::Init();
	// LOAD SOUND EFFECTS
	static uint32_t sound_zap = ShortSound::AddShortSound("../assets/sounds/zap15.ogg");
	static ShortSound zap_source;
	//zap_source.SetVolume(1.1f);
	static uint32_t sound_hit_ast = ShortSound::AddShortSound("../assets/sounds/shot2.ogg");
	static AA::ShortSound astroid_hit_source;
	//astroid_hit_source.SetVolume(1.8f);
	astroid_hit_source.SetRelative(1);
	SoundListener::Get()->SetDistanceModel(AL_NONE);

	// LOAD MUSIC
	static AA::LongSound music_source("../assets/sounds/music/Interplanetary Alignment - NoMBe (stereo).ogg");
	music_source.SetVolume(.1f);

	auto startupSettings = []()
	{
		music_source.Play();
	};
	LOOP->addToOnBegin(startupSettings);

	auto updateMusic = []() {
		music_source.UpdatePlayBuffer();
	};
	LOOP->addToSlowUpdate(updateMusic);

	static int unlit_shader = LOOP->addShader("../assets/shaders/noLight.vert", "../assets/shaders/noLight.frag");

	static int lit_shader = LOOP->addShader("../assets/shaders/combinedLight.vert", "../assets/shaders/combinedLight.frag");
	// directional light for lit shader
	static AA::DirectionalLight directional_light{};
	directional_light.Direction = glm::vec3(-0.45f, -1.f, 0.f);
	directional_light.Ambient = glm::vec3(0.05f);
	directional_light.Diffuse = glm::vec3(0.3f);
	directional_light.Specular = glm::vec3(1);
	setDirectionalLight(directional_light, LOOP->getShader(lit_shader));

	// BACKDROP
	static int starplane_object = LOOP->addObject("../assets/models/obj/starplane.obj", cam_1, lit_shader);
	LOOP->getGameObject(starplane_object).translateTo(glm::vec3(0, -99, 0));
	LOOP->getGameObject(starplane_object).scaleTo(glm::vec3(10, 1, 10));

	// PLAYER SHIP
	static int player_ship_object = LOOP->addObject("../assets/models/obj/flyingV.obj", cam_1, unlit_shader);
	LOOP->getGameObject(player_ship_object).translateTo(glm::vec3(0, -20, 0));
	LOOP->getGameObject(player_ship_object).scaleTo(glm::vec3(.6667f));
	LOOP->getGameObject(player_ship_object).rotateTo(glm::vec3(0, glm::radians(180.f), 0));

	// LAZER BULLET
	static int bullet_object = LOOP->addObject("../assets/models/obj/lazer.obj", cam_1, unlit_shader);
	LOOP->getGameObject(bullet_object).translateTo(LOOP->getGameObject(player_ship_object).getLocation());
	LOOP->getGameObject(bullet_object).scaleTo(glm::vec3(.3333f));
	LOOP->getGameObject(bullet_object).rotateTo(glm::vec3(0, glm::radians(180.f), 0));
	LOOP->getGameObject(bullet_object).setColliderSphere(LOOP->getGameObject(player_ship_object).getLocation(), .02f);

	// ASTROIDS
	static int go_asteroid = LOOP->addObject("../assets/models/obj/asteroid.obj", cam_1, unlit_shader);
	LOOP->getGameObject(go_asteroid).translateTo(glm::vec3(0, -20, -5));
	LOOP->getGameObject(go_asteroid).setColliderSphere(glm::vec3(0, -20, -5), 1.f);

	static int go_asteroid2 = LOOP->addObject("../assets/models/obj/asteroid2.obj", cam_1, unlit_shader);
	LOOP->getGameObject(go_asteroid2).translateTo(glm::vec3(-7, -20, -7));
	LOOP->getGameObject(go_asteroid2).setColliderSphere(glm::vec3(-7, -20, -7), 1.f);

	static std::vector<Astroid> astroids;
	astroids.push_back(createAstroid(go_asteroid, 0, false));
	astroids.push_back(createAstroid(go_asteroid2, 0, false));


	static bool turnleft(false), turnright(false), moveforward(false), fireweap(false);
	auto hotkeys = [](AA::KeyboardInput& keypress)
	{
		if (keypress.w || keypress.upArrow)  // move in pointing direction
		{
			moveforward = true;
		}
		else if (!(keypress.w || keypress.upArrow))
		{
			moveforward = false;
		}

		if (keypress.a || keypress.leftArrow)  // turn left
		{
			turnleft = true;
		}
		else if (!(keypress.a || keypress.leftArrow))
		{
			turnleft = false;
		}

		if (keypress.d || keypress.rightArrow)  // turn right
		{
			turnright = true;
		}
		else if (!(keypress.d || keypress.rightArrow))
		{
			turnright = false;
		}

		if (keypress.spacebar || keypress.mouseButton1)
		{
			fireweap = true;
		}
		else if (!(keypress.spacebar || keypress.mouseButton1))
		{
			fireweap = false;
		}
	};
	LOOP->addToKeyHandling(hotkeys);

	static const float BOUNDRYSIZE = 11.1f;

	const float TURNSPEED = 210.f;
	static const float TURNSPEEDr = glm::radians(TURNSPEED);
	static const float MOVESPEED = 6.9f;
	static const float BULLETCOOLDOWN = .4187f;
	static const float BULLETSPEED = 21.f;

	static float xShipDir = 0.0;
	static float zShipDir = 0.0;
	static float xCamPos = 0.0;
	static float zCamPos = 0.0;
	static float xFireDir = 0.0;
	static float zFireDir = 0.0;

	static bool bulletOut(false);
	static bool bulletHitSomething(false);

	static bool bulletOnCooldown(false);
	static float bulletCooldownLength = 0.f;

	static auto resetBullet = []() {
		// reset variables
		bulletOut = false; bulletHitSomething = false;
		// reset location
		LOOP->getGameObject(bullet_object).translateTo(LOOP->getGameObject(player_ship_object).getLocation());
		LOOP->getGameObject(bullet_object).rotateTo(LOOP->getGameObject(player_ship_object).getRotation());
	};

	auto controlShip = [](float dt) {
		if (moveforward)
		{
			glm::vec3 dir = LOOP->getGameObject(player_ship_object).getRotation();
			xShipDir = sin(dir.y);
			zShipDir = cos(dir.y);
			LOOP->getGameObject(player_ship_object).advanceTranslate(glm::vec3(xShipDir, 0, zShipDir) * dt * MOVESPEED);

			//positive bounds
			if (LOOP->getGameObject(player_ship_object).getLocation().x > BOUNDRYSIZE)
				LOOP->getGameObject(player_ship_object).advanceTranslate(glm::vec3(-BOUNDRYSIZE * 2, 0, 0));

			if (LOOP->getGameObject(player_ship_object).getLocation().z > BOUNDRYSIZE)
				LOOP->getGameObject(player_ship_object).advanceTranslate(glm::vec3(0, 0, -BOUNDRYSIZE * 2));

			//negative bounds
			if (LOOP->getGameObject(player_ship_object).getLocation().x < -BOUNDRYSIZE)
				LOOP->getGameObject(player_ship_object).advanceTranslate(glm::vec3(BOUNDRYSIZE * 2, 0, 0));

			if (LOOP->getGameObject(player_ship_object).getLocation().z < -BOUNDRYSIZE)
				LOOP->getGameObject(player_ship_object).advanceTranslate(glm::vec3(0, 0, BOUNDRYSIZE * 2));

			// set listener location for positional sound calc
			SoundListener::Get()->SetPosition(LOOP->getGameObject(player_ship_object).getLocation());

			if (!bulletOut)
			{
				//apply loc to bullet if it is not out
				LOOP->getGameObject(bullet_object).translateTo(LOOP->getGameObject(player_ship_object).getLocation());
			}
		}

		if (turnleft)
		{
			LOOP->getGameObject(player_ship_object).advanceRotation(glm::vec3(0, TURNSPEEDr * dt, 0));

			// set listener orientation for positional sound calc

			//const glm::mat4 inverted = glm::inverse(LOOP->getGameObject(player_ship_object).getModelMatrix(0));
			//const glm::vec3 forward = normalize(glm::vec3(inverted[2]));

			//glm::vec3 dir = LOOP->getGameObject(player_ship_object).getRotation();
			//xShipDir = sin(dir.y);
			//zShipDir = cos(dir.y);
			SoundListener::Get()->SetOrientation(glm::vec3(xShipDir, 0, zShipDir),	glm::vec3(0, 1, 0));

			if (!bulletOut)
			{
				//apply rotation to bullet if it is not out
				LOOP->getGameObject(bullet_object).rotateTo(LOOP->getGameObject(player_ship_object).getRotation());
			}
		}

		if (turnright)
		{
			LOOP->getGameObject(player_ship_object).advanceRotation(glm::vec3(0, -TURNSPEEDr * dt, 0));

			// set listener orientation for positional sound calc
			//glm::vec3 dir = LOOP->getGameObject(player_ship_object).getRotation();
			//glm::vec3 front{};
			//front.x = cos(dir.y);
			//front.y = 0;
			//front.z = sin(dir.y);
			SoundListener::Get()->SetOrientation(glm::vec3(xShipDir, 0, zShipDir), glm::vec3(0, 1, 0));

			if (!bulletOut)
			{
				//apply rotation to bullet if it is not out
				LOOP->getGameObject(bullet_object).rotateTo(LOOP->getGameObject(player_ship_object).getRotation());
			}
		}

		// bullet logic
		if (fireweap && !bulletOnCooldown)
		{
			bulletOut = true;
			bulletOnCooldown = true;
			glm::vec3 dir = LOOP->getGameObject(player_ship_object).getRotation();
			xShipDir = sin(dir.y);
			zShipDir = cos(dir.y);
			xFireDir = xShipDir;
			zFireDir = zShipDir;
			zap_source.Play(sound_zap);
		}

		if (bulletOnCooldown)
		{
			bulletCooldownLength += dt;

			if (bulletCooldownLength > BULLETCOOLDOWN)
			{
				bulletOnCooldown = false;
				bulletCooldownLength = 0.f;
				if (bulletOut)  //otherwise it is already reset
				{
					resetBullet();
				}
			}

			if (bulletOut)
			{
				if (!bulletHitSomething)
				{
					LOOP->getGameObject(bullet_object).advanceTranslate(glm::vec3(xFireDir, 0, zFireDir) * dt * BULLETSPEED);

					//positive bounds
					if (LOOP->getGameObject(bullet_object).getLocation().x > BOUNDRYSIZE)
						LOOP->getGameObject(bullet_object).advanceTranslate(glm::vec3(-BOUNDRYSIZE * 2, 0, 0));

					if (LOOP->getGameObject(bullet_object).getLocation().z > BOUNDRYSIZE)
						LOOP->getGameObject(bullet_object).advanceTranslate(glm::vec3(0, 0, -BOUNDRYSIZE * 2));

					//negative bounds
					if (LOOP->getGameObject(bullet_object).getLocation().x < -BOUNDRYSIZE)
						LOOP->getGameObject(bullet_object).advanceTranslate(glm::vec3(BOUNDRYSIZE * 2, 0, 0));

					if (LOOP->getGameObject(bullet_object).getLocation().z < -BOUNDRYSIZE)
						LOOP->getGameObject(bullet_object).advanceTranslate(glm::vec3(0, 0, BOUNDRYSIZE * 2));
				}
			}
		}
	};
	LOOP->addToDeltaUpdate(controlShip);

	auto checkCollide = []()
	{
		if (bulletOut && !bulletHitSomething) {
			for (auto& ast : astroids)
			{
				if (AA::CollisionHandler::getInstance()->sphere_vs_Sphere_3D
				(
					LOOP->getGameObject(bullet_object).getColliderSphere(),
					LOOP->getGameObject(ast.object_id).getColliderSphere(ast.instance_id)
				))
				{
					resetBullet();
					astroid_hit_source.SetLocation(LOOP->getGameObject(ast.object_id).getLocation());
					hitAstroid(ast, astroids);
					astroid_hit_source.Play(sound_hit_ast);
					return;
				}
			}
		}
	};
	LOOP->addToUpdate(checkCollide);

	static const float STARTDELAY = 3.f;
	auto moveasteroids = [](float dt) {
		static float live_timer = 0.f;
		live_timer += dt;
		if (live_timer < STARTDELAY)
			return;
		for (const auto& ast : astroids)
		{
			//positive bounds
			if (LOOP->getGameObject(ast.object_id).getLocation(ast.instance_id).x > BOUNDRYSIZE)
				LOOP->getGameObject(ast.object_id).advanceTranslate(glm::vec3(-BOUNDRYSIZE * 2, 0, 0), ast.instance_id);

			if (LOOP->getGameObject(ast.object_id).getLocation(ast.instance_id).z > BOUNDRYSIZE)
				LOOP->getGameObject(ast.object_id).advanceTranslate(glm::vec3(0, 0, -BOUNDRYSIZE * 2), ast.instance_id);

			//negative bounds
			if (LOOP->getGameObject(ast.object_id).getLocation(ast.instance_id).x < -BOUNDRYSIZE)
				LOOP->getGameObject(ast.object_id).advanceTranslate(glm::vec3(BOUNDRYSIZE * 2, 0, 0), ast.instance_id);

			if (LOOP->getGameObject(ast.object_id).getLocation(ast.instance_id).z < -BOUNDRYSIZE)
				LOOP->getGameObject(ast.object_id).advanceTranslate(glm::vec3(0, 0, BOUNDRYSIZE * 2), ast.instance_id);

			// standard movement
			LOOP->getGameObject(ast.object_id).advanceTranslate(glm::vec3(dt * ast.direction.x * ast.speed, 0, dt * ast.direction.y * ast.speed), ast.instance_id);
			LOOP->getGameObject(ast.object_id).advanceRotation(glm::vec3(dt * ast.rotation.x, dt * ast.rotation.y, dt * ast.rotation.z), ast.instance_id);
		}
	};
	LOOP->addToDeltaUpdate(moveasteroids);

	return LOOP->runMainLoop();
}
