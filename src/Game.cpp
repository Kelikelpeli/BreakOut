#include <algorithm>
#include <sstream>
#include <filesystem>

#include "game.h"
#include "Managers/resource_manager.h"
#include "sprite_renderer.h"
#include "ballObject.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "text_renderer.h"

#include <irrklang/irrKlang.h>
using namespace irrklang;

// Game-related State data
SpriteRenderer* Renderer;
GameObject* Player;
//BallObject* Ball;
ParticleGenerator* Particles;
PostProcessor* Effects;
ISoundEngine* SoundEngine = createIrrKlangDevice();
TextRenderer* Text;

float ShakeTime = 0.0f;

Game::Game(unsigned int width, unsigned int height)
	: State(GAME_MENU), Keys(), KeysProcessed(), Width(width), Height(height), Level(0), Lives(3), Split(false), Countdown(COUNTDOWN_START), ExtraLifeCounter(BLOCK_COUNT_LIFES)
{

}

Game::~Game()
{
	delete Renderer;
	delete Player;
	//delete Ball;
	delete Particles;
	delete Effects;
	delete Text;
	SoundEngine->drop();
}

void Game::Init()
{
	// load shaders
	ResourceManager::LoadShader("src/shaders/sprite.vs", "src/shaders/sprite.fs", nullptr, "sprite");
	ResourceManager::LoadShader("src/shaders/particle.vs", "src/shaders/particle.fs", nullptr, "particle");
	ResourceManager::LoadShader("src/shaders/post_processing.vs", "src/shaders/post_processing.fs", nullptr, "postprocessing");

	//configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("particle").SetMatrix4("projection", projection);


	// load textures
	ResourceManager::LoadTexture("src/resources/textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("src/resources/textures/awesomeface.png", true, "face");
	ResourceManager::LoadTexture("src/resources/textures/block.png", false, "block");
	ResourceManager::LoadTexture("src/resources/textures/block_solid.png", false, "block_solid");
	ResourceManager::LoadTexture("src/resources/textures/paddle.png", true, "paddle");
	ResourceManager::LoadTexture("src/resources/textures/particle.png", true, "particle");
	ResourceManager::LoadTexture("src/resources/textures/powerup_speed.png", true, "powerup_speed");
	ResourceManager::LoadTexture("src/resources/textures/powerup_sticky.png", true, "powerup_sticky");
	ResourceManager::LoadTexture("src/resources/textures/powerup_increase.png", true, "powerup_increase");
	ResourceManager::LoadTexture("src/resources/textures/powerup_confuse.png", true, "powerup_confuse");
	ResourceManager::LoadTexture("src/resources/textures/powerup_chaos.png", true, "powerup_chaos");
	ResourceManager::LoadTexture("src/resources/textures/powerup_passthrough.png", true, "powerup_passthrough");

	//Power_Up extra
	ResourceManager::LoadTexture("src/resources/textures/powerup_split.png", true, "powerup_split");

	// set render-specific controls
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
	Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);
	Text = new TextRenderer(this->Width, this->Height);
	Text->Load("src/resources/fonts/ocraext.TTF", 24);

	// load levels
	GameLevel one; one.Load("src/resources/levels/one.lvl", this->Width, this->Height / 2);
	GameLevel two; two.Load("src/resources/levels/two.lvl", this->Width, this->Height / 2);
	GameLevel three; three.Load("src/resources/levels/three.lvl", this->Width, this->Height / 2);
	GameLevel four; four.Load("src/resources/levels/four.lvl", this->Width, this->Height / 2);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;

	// configure game objects
	glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
	//Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
	Balls.clear(); // Asegurarse de que esté vacío
	Balls.emplace_back(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));


	SoundEngine->play2D("src/resources/audio/breakout.mp3", true);

	this->Countdown = COUNTDOWN_START;  //starts the countdown
	this->ExtraLifeCounter = BLOCK_COUNT_LIFES; // Restars the conter fr the extra life
}

void Game::Update(float dt)
{
	//starts the couuntdown
	if (this->State == GAME_ACTIVE&&this->Countdown >0.0f) {
		this->Countdown -= dt;
	}
	for (auto it = Balls.begin(); it != Balls.end();)
	{
		if (!it->Stuck)
		{
			it->Move(dt, this->Width);
			if (it->Position.y >= this->Height)
			{
				it = Balls.erase(it); 	 //delete the balls out of the limits

				continue;             
			}
		}
		++it; 
	}
	if (Balls.size() == 1) {
		Balls.back().Color = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	// check for collisions
	this->DoCollisions();
	// update particles
	if (!Balls.empty())
	{
		Particles->Update(dt, Balls[0], 2, glm::vec2(Balls[0].Radius / 2.0f));
	}
	// update PowerUps
	this->UpdatePowerUps(dt);
	// reduce shake time
	if (ShakeTime > 0.0f)
	{
		ShakeTime -= dt;
		if (ShakeTime <= 0.0f)
			Effects->Shake = false;
	}
	// check loss condition
	if (Balls.empty() || this->Countdown < 0.0f) // did ball reach bottom edge? did the time ends?
	{
		--this->Lives;
		// did the player lose all his lives? : Game over
		if (this->Lives == 0 || this->Countdown<0.0f)
		{
			this->ResetLevel();
			this->State = GAME_MENU;
		}
		this->ResetPlayer();
	}
	// check win condition
	if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
	{
		this->ResetLevel();
		this->ResetPlayer();
		Effects->Chaos = true;
		this->State = GAME_WIN;
	}
}

void Game::ProcessInput(float dt)
{
	if (this->State == GAME_MENU)
	{
		if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
		{
			this->State = GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
		}
		if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
		{
			this->Level = (this->Level + 1) % 4;
			this->KeysProcessed[GLFW_KEY_W] = true;
		}
		if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
		{
			if (this->Level > 0)
				--this->Level;
			else
				this->Level = 3;
			//this->Level = (this->Level - 1) % 4;
			this->KeysProcessed[GLFW_KEY_S] = true;
		}
	}
	if (this->State == GAME_WIN)
	{
		if (this->Keys[GLFW_KEY_ENTER])
		{
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
			Effects->Chaos = false;
			this->State = GAME_MENU;
		}
	}
	if (this->State == GAME_ACTIVE)
	{
		float velocity = PLAYER_VELOCITY * dt;
		// move playerboard
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0.0f)
			{
				Player->Position.x -= velocity;
				for (auto& ball : Balls)
				{
					if (ball.Stuck)
						ball.Position.x -= velocity;
				}
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
				for (auto& ball : Balls)
				{
					if (ball.Stuck)
						ball.Position.x += velocity;
				}
			}
		}
		if (this->Keys[GLFW_KEY_SPACE])
		{
			for (auto& ball : Balls)
			{
				if (ball.Stuck)
				{
					ball.Stuck = false;
					ball.Velocity = INITIAL_BALL_VELOCITY;
					break;
				}
			}
		}

		//para testear el win
	/*	if (this->Keys[GLFW_KEY_U]) {
			for (GameObject& brick : this->Levels[this->Level].Bricks)
			{
				if (!brick.IsSolid)
					brick.Destroyed = true;
			}
		}*/
	}
}

void Game::Render()
{

	if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN)
	{
		// begin rendering to postprocessing framebuffer
		Effects->BeginRender();
		// draw background
		Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
		// draw level
		this->Levels[this->Level].Draw(*Renderer);
		// draw player
		Player->Draw(*Renderer);
		// draw PowerUps
		for (PowerUp& powerUp : this->PowerUps)
			if (!powerUp.Destroyed)
				powerUp.Draw(*Renderer);
		// draw particles	
		Particles->Draw();
		// draw ball
		for (BallObject& ball : Balls)			
			ball.Draw(*Renderer);
		// end rendering to postprocessing framebuffer
		Effects->EndRender();
		// render postprocessing quad
		Effects->Render(glfwGetTime());
		// render text (don't include in postprocessing)
		std::stringstream ss; ss << this->Lives;
		Text->RenderText("Lives:" + ss.str(), 26.0f, 10.0f, 1.0f);


		// The extras life
		std::stringstream counterText;
		counterText << "Hits: " << 10-this->ExtraLifeCounter << "/10";
		Text->RenderText(counterText.str(), 26.0f, 30.0f, 0.7f, glm::vec3(1.0f, 1.0f, 1.0f));

		std::stringstream countdownText;
		countdownText << "Time: " << static_cast<int>(this->Countdown);
		Text->RenderText(countdownText.str(), this->Width - 170.0f, 10.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

	}
	if (this->State == GAME_MENU)
	{
		Text->RenderText("Press ENTER to start", 250.0f, this->Height / 2.0f, 1.0f);
		Text->RenderText("Press W or S to select level", 245.0f, this->Height / 2.0f + 20.0f, 0.75f);
	}
	if (this->State == GAME_WIN)
	{
		Text->RenderText("You WON!!!", 320.0f, this->Height / 2.0f - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Text->RenderText("Press ENTER to retry or ESC to quit", 130.0f, this->Height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
	}
}

void Game::ResetLevel()
{
	if (this->Level == 0)
		this->Levels[0].Load("src/resources/levels/one.lvl", this->Width, this->Height / 2);
	else if (this->Level == 1)
		this->Levels[1].Load("src/resources/levels/two.lvl", this->Width, this->Height / 2);
	else if (this->Level == 2)
		this->Levels[2].Load("src/resources/levels/three.lvl", this->Width, this->Height / 2);
	else if (this->Level == 3)
		this->Levels[3].Load("src/resources/levels/four.lvl", this->Width, this->Height / 2);

	this->Lives = 3;
	this->Countdown = COUNTDOWN_START;
}

void Game::ResetPlayer()
{
	
	// reset player/ball stats
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	//then create a ball
	glm::vec2 ballPos = Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
	Balls.emplace_back(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
	Balls.back().Stuck = true;
	Balls.back().PassThrough = false;
	//Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
	// also disable all active powerups
	Effects->Confuse = false;
	Effects->Chaos = false;
	Player->Color = glm::vec3(1.0f);
	Balls.back().Color = glm::vec3(1.0f);
	
	this->ExtraLifeCounter = BLOCK_COUNT_LIFES;
}
// powerups
bool IsOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type);


void Game::UpdatePowerUps(float dt)
{
	for (PowerUp& powerUp : this->PowerUps)
	{
		powerUp.Position += powerUp.Velocity * dt;
		if (powerUp.Activated)
		{
			powerUp.Duration -= dt;

			if (powerUp.Duration <= 0.0f)
			{
				// remove powerup from list (will later be removed)
				powerUp.Activated = false;
				// deactivate effects
				if (powerUp.Type == "sticky")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
					{
						// only reset if no other PowerUp of type sticky is active						
						for (auto& ball : Balls)
						{
							ball.Sticky = false;
						}
						Player->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "pass-through")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
					{	// only reset if no other PowerUp of type pass-through is active
						for (auto& ball : Balls)
						{
							ball.PassThrough = false;
							ball.Color = glm::vec3(1.0f);
						}
					}
				}
				else if (powerUp.Type == "confuse")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
					{	// only reset if no other PowerUp of type confuse is active
						Effects->Confuse = false;
					}
				}
				else if (powerUp.Type == "chaos")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
					{	// only reset if no other PowerUp of type chaos is active
						Effects->Chaos = false;
					}
				}

				//extra power_up
				else if (powerUp.Type == "split")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "split"))
						if (Balls.size() == 1)
						{	// only reset if no other PowerUp of type split is active
							this->Split = false;
							while (Balls.size() > 1)
							{
								Balls.pop_back();
							}
						}			
				
				}
			}
		}
	}

// Remove all PowerUps from vector that are destroyed AND !activated (thus either off the map or finished)
// Note we use a lambda expression to remove each PowerUp which is destroyed and not activated
this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
	[](const PowerUp& powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
), this->PowerUps.end());
}

bool ShouldSpawn(unsigned int chance)
{
	unsigned int random = rand() % chance;
	return random == 0;
}
void Game::SpawnPowerUps(GameObject& block)
{
	if (ShouldSpawn(75)) // 1 in 75 chance
		this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_speed")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, ResourceManager::GetTexture("powerup_increase")));
	if (ShouldSpawn(15)) // Negative powerups should spawn more often
		this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")));
	if (ShouldSpawn(15))
		this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")));

	//power_up extra
	if (ShouldSpawn(5))
	{
		this->PowerUps.push_back(PowerUp("split", glm::vec3(0.0f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_split")));
	}
}

void Game::ActivatePowerUp(PowerUp& powerUp)
{
	if (powerUp.Type == "speed")
	{
		for (auto& ball : Balls)
		
		ball.Velocity *= 1.2;
	}
	else if (powerUp.Type == "sticky")
	{
		for (auto& ball : Balls) {
			ball.Sticky = true;
		}
		Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	}
	else if (powerUp.Type == "pass-through")
	{
		for (auto& ball : Balls) {
			ball.PassThrough = true;
			ball.Color = glm::vec3(1.0f, 0.5f, 0.5f);
		}
		
	}
	else if (powerUp.Type == "pad-size-increase")
	{
		Player->Size.x += 50;
	}
	else if (powerUp.Type == "confuse")
	{
		if (!Effects->Chaos)
			Effects->Confuse = true; // only activate if chaos wasn't already active
	}
	else if (powerUp.Type == "chaos")
	{
		if (!Effects->Confuse)
			Effects->Chaos = true;
	}
	//Power_Up extra
	else if (powerUp.Type == "split")
	{
		//chack if already are more than 1+one ball in the vector
		if (Balls.size() >= 2)
			return;
		//else create the balls
		this->Split = true;

		//the current position of the player
		glm::vec2 ballPos = Player->Position + glm::vec2(Player->Size.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
		for (int i = 0; i < 2; ++i)
		{
			//set new velocity
			glm::vec2 velocity = (i == 0)
				? glm::vec2(-INITIAL_BALL_VELOCITY.x, INITIAL_BALL_VELOCITY.y)
				: glm::vec2(INITIAL_BALL_VELOCITY.x, INITIAL_BALL_VELOCITY.y);
			//add one ball
			Balls.emplace_back(ballPos, BALL_RADIUS, velocity, ResourceManager::GetTexture("face"));
			Balls.back().Color = glm::vec3(1.0f, 0.0f, 0.0f);
		}
	}
}

bool IsOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type)
{
	// Check if another PowerUp of the same type is still active
	// in which case we don't disable its effect (yet)
	for (const PowerUp& powerUp : powerUps)
	{
		if (powerUp.Activated)
			if (powerUp.Type == type)
				return true;
	}
	return false;
}


bool CheckCollision(GameObject& one, GameObject& two);
Collision CheckCollision(BallObject& one, GameObject& two);
Direction VectorDirection(glm::vec2 closest);

void Game::DoCollisions()
{
	for (auto& ball : Balls)
	{

		for (GameObject& box : this->Levels[this->Level].Bricks)
		{
			if (!box.Destroyed)
			{

				Collision collision = CheckCollision(ball, box);
				if (std::get<0>(collision)) // if collision is true
				{
					// destroy block if not solid
					if (!box.IsSolid)
					{
						box.Destroyed = true;
						this->ExtraLifeCounter--;
						this->SpawnPowerUps(box);
						SoundEngine->play2D("src/resources/audio/bleep.mp3", false);
						if (ExtraLifeCounter <1) {
							this->Lives++;
							ExtraLifeCounter = BLOCK_COUNT_LIFES;
						}
					}
					else
					{   // if block is solid, enable shake effect
						ShakeTime = 0.05f;
						Effects->Shake = true;
						this->ExtraLifeCounter = BLOCK_COUNT_LIFES;
						SoundEngine->play2D("src/resources/audio/solid.wav", false);
					}

					// collision resolution
					Direction dir = std::get<1>(collision);
					glm::vec2 diff_vector = std::get<2>(collision);
					if (!(ball.PassThrough && !box.IsSolid))
					{
						if (dir == LEFT || dir == RIGHT) // horizontal collision
						{
							ball.Velocity.x = -ball.Velocity.x; // reverse horizontal velocity
							// relocate
							float penetration = ball.Radius - std::abs(diff_vector.x);
							if (dir == LEFT)
								ball.Position.x += penetration; // move ball to right
							else
								ball.Position.x -= penetration; // move ball to left;
						}
						else // vertical collision
						{
							ball.Velocity.y = -ball.Velocity.y; // reverse vertical velocity
							// relocate
							float penetration = ball.Radius - std::abs(diff_vector.y);
							if (dir == UP)
								ball.Position.y -= penetration; // move ball back up
							else
								ball.Position.y += penetration; // move ball back down
						}
					}
				}
			}


		}

		for (PowerUp& powerUp : this->PowerUps)
		{
			if (!powerUp.Destroyed)
			{
				if (powerUp.Position.y >= this->Height)
					powerUp.Destroyed = true;
				if (CheckCollision(*Player, powerUp))
				{	// collided with player, now activate powerup
					ActivatePowerUp(powerUp);
					powerUp.Destroyed = true;
					powerUp.Activated = true;
					SoundEngine->play2D("src/resources/audio/powerup.wav", false);
				}
			}
		}

		// check collisions for player pad (unless stuck)
		Collision result = CheckCollision(ball, *Player);
		if (!ball.Stuck && std::get<0>(result))
		{
			// check where it hit the board, and change velocity based on where it hit the board
			float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
			float distance = (ball.Position.x + ball.Radius) - centerBoard;
			float percentage = distance / (Player->Size.x / 2.0f);
			// then move accordingly
			float strength = 2.0f;
			glm::vec2 oldVelocity = ball.Velocity;
			ball.Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
			//Ball->Velocity.y = -Ball->Velocity.y;
			ball.Velocity = glm::normalize(ball.Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
			// fix sticky paddle
			ball.Velocity.y = -1.0f * abs(ball.Velocity.y);
			ball.Stuck = ball.Sticky;

			SoundEngine->play2D("src/resources/audio/bleep.wav", false);
		}
	}
}

bool CheckCollision(GameObject& one, GameObject& two) // AABB - AABB collision
{
	// collision x-axis?
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
		two.Position.x + two.Size.x >= one.Position.x;
	// collision y-axis?
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
		two.Position.y + two.Size.y >= one.Position.y;
	// collision only if on both axes
	return collisionX && collisionY;
}

Collision CheckCollision(BallObject& one, GameObject& two) // AABB - Circle collision
{
	if (&one == nullptr) 
		return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
	// get center point circle first 
	glm::vec2 center(one.Position + one.Radius);
	// calculate AABB info (center, half-extents)
	glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	// get difference vector between both centers
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// now that we know the clamped values, add this to AABB_center and we get the value of box closest to circle
	glm::vec2 closest = aabb_center + clamped;
	// now retrieve vector between center circle and closest point AABB and check if length < radius
	difference = closest - center;

	if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
		return std::make_tuple(true, VectorDirection(difference), difference);
	else
		return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),	// up
		glm::vec2(1.0f, 0.0f),	// right
		glm::vec2(0.0f, -1.0f),	// down
		glm::vec2(-1.0f, 0.0f)	// left
	};
	float max = 0.0f;
	unsigned int best_match = -1;
	for (unsigned int i = 0; i < 4; i++)
	{
		float dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}
