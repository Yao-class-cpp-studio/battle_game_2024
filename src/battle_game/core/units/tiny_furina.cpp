#include "tiny_Furina.h"

#include "battle_game/core/bullets/bullets.h"
#include "battle_game/core/game_core.h"
#include "battle_game/graphics/graphics.h"

namespace battle_game::unit {

namespace {
uint32_t furina_hat_model_index = 0xffffffffu;
uint32_t furina_body_model_index = 0xffffffffu;
uint32_t furina_guys_model_index = 0xffffffffu;
uint32_t furina_bow_model_index = 0xffffffffu;
}   // namespace

Furina::Furina(GameCore *game_core, uint32_t id, uint32_t player_id)
    : Unit(game_core, id, player_id) {
  if (!~furina_body_model_index) {
    auto mgr = AssetsManager::GetInstance();
    {
      // Furina's hat
      furina_hat_model_index = mgr->RegisterModel(
        /* Brim vertices */
       {
        /* Vertex data (position, texture coordinates, color) */
        {{-0.8f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},  // Bottom left corner
        {{-0.6f, 0.2f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},  // Left middle
        {{-0.4f, 0.4f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},  // Left brim
        {{0.0f, 0.6f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},   // Top point
        {{0.4f, 0.4f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},   // Right brim
        {{0.6f, 0.2f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},   // Right middle
        {{0.8f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},   // Bottom right corner
        {{0.6f, -0.2f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},  // Bottom right inner
        {{-0.6f, -0.2f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}, // Bottom left inner
        },
    /* Vertex indices */
    {0, 1, 8, 1, 2, 8, 2, 3, 4, 2, 4, 7, 4, 5, 7, 5, 6, 7});
    }

    {
      // Furina's body
      std::vector<ObjectVertex> body_vertices;
      std::vector<uint32_t> body_indices;
      const int precision = 60;
      const float inv_precision = 1.0f / float(precision);
      for (int i = 0; i < precision; i++) {
      auto theta = (float(i) + 0.5f) * inv_precision;
      theta *= glm::pi<float>() * 2.0f;
      auto sin_theta = std::sin(theta);
      auto cos_theta = std::cos(theta);
      body_vertices.push_back({{sin_theta * 0.9f, cos_theta * 0.9f},
                   {0.0f, 0.0f},
                   {1.0f, 1.0f, 1.0f, 1.0f}});
      body_indices.push_back(i);
      body_indices.push_back((i + 1) % precision);
      body_indices.push_back(precision);
      }
      furina_body_model_index = mgr->RegisterModel(body_vertices, body_indices);
    }

    {
        // Furina's bow
      // Add a pink bow
      std::vector<ObjectVertex> bow_vertices;
      std::vector<uint32_t> bow_indices;

      // Bow center (pink color)
      bow_vertices.push_back(
          {{0.0f, 1.1f}, {0.0f, 0.0f}, {1.0f, 0.7f, 0.8f, 1.0f}});  // Center

      // Left triangle (left side of bow)
      bow_vertices.push_back(
          {{-0.2f, 1.3f}, {0.0f, 0.0f}, {1.0f, 0.7f, 0.8f, 1.0f}});
      bow_vertices.push_back(
          {{-0.4f, 1.1f}, {0.0f, 0.0f}, {1.0f, 0.7f, 0.8f, 1.0f}});
      bow_vertices.push_back(
          {{-0.2f, 0.9f}, {0.0f, 0.0f}, {1.0f, 0.7f, 0.8f, 1.0f}});

      // Right triangle (right side of bow)
      bow_vertices.push_back(
          {{0.2f, 1.3f}, {0.0f, 0.0f}, {1.0f, 0.7f, 0.8f, 1.0f}});
      bow_vertices.push_back(
          {{0.4f, 1.1f}, {0.0f, 0.0f}, {1.0f, 0.7f, 0.8f, 1.0f}});
      bow_vertices.push_back(
          {{0.2f, 0.9f}, {0.0f, 0.0f}, {1.0f, 0.7f, 0.8f, 1.0f}});

      // Left triangle indices
      bow_indices.push_back(0);  // Center
      bow_indices.push_back(1);  // Top left
      bow_indices.push_back(2);  // Bottom left
      bow_indices.push_back(0);
      bow_indices.push_back(2);
      bow_indices.push_back(3);
      // Right triangle indices
      bow_indices.push_back(0);  // Center
      bow_indices.push_back(4);  // Top right
      bow_indices.push_back(5);  // Bottom right
      bow_indices.push_back(0);
      bow_indices.push_back(5);
      bow_indices.push_back(6);

      // Register the bow model
      furina_bow_model_index = mgr->RegisterModel(bow_vertices, bow_indices);
    }

    {
      /* Furina Guys */
      /* Tank Turret: Represents the conical top of the hat with gradient color
       */
      std::vector<ObjectVertex> turret_vertices;
      std::vector<uint32_t> turret_indices;
      const int precision = 30;   // Precision for the circular base of the cone
      const float radius = 0.5f;  // Radius of the base
      const float height = 1.0f;  // Height of the cone

      // Generate circular base of the cone with gradient color
      for (int i = 0; i < precision; i++) {
        float theta = (float(i) / float(precision)) * glm::pi<float>() * 2.0f;
        float x = std::cos(theta) * radius;
        float y = std::sin(theta) * radius;

        // Bottom vertices with blue color
        turret_vertices.push_back(
            {{x, y}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}});  // Blue
        turret_indices.push_back(i);
        turret_indices.push_back((i + 1) % precision);
        turret_indices.push_back(precision);
      }

      // Add the center of the base with blue color
      turret_vertices.push_back({{0.0f, 0.0f},
                                 {0.0f, 0.0f},
                                 {0.0f, 0.0f, 1.0f, 1.0f}});  // Blue center

      // Add the cone tip with a gradient color (white or light blue)
      turret_vertices.push_back({{0.0f, height},
                                 {0.0f, 0.0f},
                                 {0.7f, 0.9f, 1.0f, 1.0f}});  // Light blue

      // Add indices to connect the tip to the base
      for (int i = 0; i < precision; i++) {
        turret_indices.push_back(i);
        turret_indices.push_back((i + 1) % precision);
        turret_indices.push_back(precision + 1);  // Cone tip
      }

      // Register the model
      furina_guys_model_index =
          mgr->RegisterModel(turret_vertices, turret_indices);

    }
  }
}

void Furina::Render() {
  battle_game::SetTransformation(position_, rotation_);
  battle_game::SetTexture(0);
  battle_game::SetColor(game_core_->GetPlayerColor(player_id_));
  battle_game::DrawModel(furina_body_model_index);
  battle_game::DrawModel(furina_bow_model_index);
  battle_game::SetRotation(guys_rotation_);
  battle_game::DrawModel(furina_guys_model_index);
  battle_game::DrawModel(furina_hat_model_index);
}

void Furina::Update() {
  FurinaMove(3.0f, glm::radians(180.0f));
  GuysRotate();
  Fire();
}

void Furina::FurinaMove(float move_speed, float rotate_angular_speed) {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    glm::vec2 offset{0.0f};
    if (input_data.key_down[GLFW_KEY_W]) {
      offset.y += 1.0f;
    }
    if (input_data.key_down[GLFW_KEY_S]) {
      offset.y -= 1.0f;
    }
    float speed = move_speed * GetSpeedScale();
    offset *= kSecondPerTick * speed;
    auto new_position =
        position_ + glm::vec2{glm::rotate(glm::mat4{1.0f}, rotation_,
                                          glm::vec3{0.0f, 0.0f, 1.0f}) *
                              glm::vec4{offset, 0.0f, 0.0f}};
    if (!game_core_->IsBlockedByObstacles(new_position)) {
      game_core_->PushEventMoveUnit(id_, new_position);
    }
    float rotation_offset = 0.0f;
    if (input_data.key_down[GLFW_KEY_A]) {
      rotation_offset += 1.0f;
    }
    if (input_data.key_down[GLFW_KEY_D]) {
      rotation_offset -= 1.0f;
    }
    rotation_offset *= kSecondPerTick * rotate_angular_speed * GetSpeedScale();
    game_core_->PushEventRotateUnit(id_, rotation_ + rotation_offset);
  }
}

void Furina::GuysRotate() {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    auto diff = input_data.mouse_cursor_position - position_;
    if (glm::length(diff) < 1e-4) {
      guys_rotation_ = rotation_;
    } else {
      guys_rotation_ = std::atan2(diff.y, diff.x) - glm::radians(90.0f);
    }
  }
}

void Furina::Fire() {
  if (fire_count_down_ == 0) {
    auto player = game_core_->GetPlayer(player_id_);
    if (player) {
      auto &input_data = player->GetInputData();
      if (input_data.mouse_button_down[GLFW_MOUSE_BUTTON_LEFT]) {
        auto velocity = Rotate(glm::vec2{0.0f, 20.0f}, guys_rotation_);
        GenerateBullet<bullet::CannonBall>(
            position_ + Rotate({0.0f, 1.2f}, guys_rotation_),
            guys_rotation_, GetDamageScale(), velocity);
        fire_count_down_ = 0.01 * kTickPerSecond;  // Fire interval 1 second.
      }
    }
  }
  if (fire_count_down_) {
    fire_count_down_--;
  }
}

bool Furina::IsHit(glm::vec2 position) const {
  position = WorldToLocal(position);
  return position.x > -0.8f && position.x < 0.8f && position.y > -1.0f &&
         position.y < 1.0f && position.x + position.y < 1.6f &&
         position.y - position.x < 1.6f;
}

const char *Furina::UnitName() const {
  return "Tiny Furina";
}

const char *Furina::Author() const {
  return "Little Furina";
}
}  // namespace battle_game::unit
