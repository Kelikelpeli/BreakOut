//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//#include <iostream>
//#include "Shader.h" // Incluye la clase Shader para manejar los sombreadores
////#define STB_IMAGE_IMPLEMENTATION
////#include "stb_image.h"
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp> // Biblioteca GLM para operaciones con matrices
#include "game.h"
#include "Managers/resource_manager.h"
#include "sprite_renderer.h"

SpriteRenderer* Renderer;

Game::Game(unsigned int width, unsigned int height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}

Game::~Game()
{

}

void Game::Init()
{
	// load shaders
	ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.fs", nullptr, "sprite");
	//configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
		static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	// set render-specific controls
	Shader spriteShader = ResourceManager::GetShader("sprite");
	Renderer = new SpriteRenderer(spriteShader);

	//Renderer = new SpriteRenderer (ResourceManager::GetShader("sprite"));
	// load textures
	ResourceManager::LoadTexture("Resources/textures/awesomeface.png", true, "face");
}

void Game::Update(float dt)
{

}

void Game::ProcessInput(float dt)
{

}

void Game::Render()
{
	Texture2D faceTexture = ResourceManager::GetTexture("face");

	Renderer->DrawSprite(faceTexture, glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}