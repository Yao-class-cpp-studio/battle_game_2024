#include "zhiyin.h"

#include "battle_game/core/bullets/bullets.h"
#include "battle_game/core/game_core.h"
#include "battle_game/graphics/graphics.h"

namespace battle_game::unit {

namespace {
uint32_t kunkun_mouth_model_index = 0xffffffffu;
uint32_t kunkun_body_model_index = 0xffffffffu;
uint32_t kunkun_eye_model_index = 0xffffffffu;

}  // namespace

Kunkun::Kunkun(GameCore *game_core, uint32_t id, uint32_t player_id)
    : Unit(game_core, id, player_id) {
  if (!~kunkun_mouth_model_index) {
    auto mgr = AssetsManager::GetInstance();
    {
      // Kunkun's mouth
      kunkun_mouth_model_index = mgr->RegisterModel(
          {{{-0.2f, 0.5657f}, {0.0f, 0.0f}, {0.742f, 0.205f, 0.125f, 1.0f}},
           {{0.2f, 0.5657f}, {0.0f, 0.0f}, {0.742f, 0.205f, 0.125f, 1.0f}},
           {{0.0f, 1.0f}, {0.0f, 0.0f}, {0.742f, 0.205f, 0.125f, 1.0f}}},
          {0, 1, 2});
    }

    {
      // Kunkun's body
      std::vector<ObjectVertex> body_vertices;
      std::vector<uint32_t> body_indices;
      const int precision = 60;
      const float inv_precision = 1.0f / float(precision);
      for (int i = 0; i < precision; i++) {
        auto theta = (float(i) + 0.5f) * inv_precision;
        theta *= glm::pi<float>() * 2.0f;
        auto sin_theta = std::sin(theta);
        auto cos_theta = std::cos(theta);
        body_vertices.push_back({{sin_theta * 0.6f, cos_theta * 0.6f},
                                 {0.0f, 0.0f},
                                 {0.747f, 0.875f, 0.2305f, 1.0f}});
        body_indices.push_back(i);
        body_indices.push_back((i + 1) % precision);
        body_indices.push_back(precision);
      }
      kunkun_body_model_index = mgr->RegisterModel(body_vertices, body_indices);
    }

    {
      // Kunkun's eye
      std::vector<ObjectVertex> eye_vertices;
      std::vector<uint32_t> eye_indices;
      const int precision = 60;
      const float inv_precision = 1.0f / float(precision);
      for (int i = 0; i < precision; i++) {
        auto theta = (float(i) + 0.5f) * inv_precision;
        theta *= glm::pi<float>() * 2.0f;
        auto sin_theta = std::sin(theta);
        auto cos_theta = std::cos(theta);
        eye_vertices.push_back({{sin_theta * 0.1f, 0.2 + cos_theta * 0.1f},
                                {0.0f, 0.0f},
                                {0.0f, 0.0f, 0.0f, 1.0f}});
        eye_indices.push_back(i);
        eye_indices.push_back((i + 1) % precision);
        eye_indices.push_back(precision);
      }
      eye_vertices.push_back(
          {{-0.0f, 0.2f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}});
      kunkun_eye_model_index = mgr->RegisterModel(eye_vertices, eye_indices);
    }
  }
}

void Kunkun::Render() {
  battle_game::SetTransformation(position_, rotation_);
  battle_game::SetTexture(0);
  battle_game::SetColor(game_core_->GetPlayerColor(player_id_));
  battle_game::DrawModel(kunkun_body_model_index);
  battle_game::DrawModel(kunkun_eye_model_index);
  battle_game::SetRotation(turret_rotation_);
  battle_game::DrawModel(kunkun_mouth_model_index);
}

void Kunkun::Update() {
  KunkunMove(3.0f, glm::radians(180.0f));
  Basketball_turret_Rotate();
  Basketball_Fire();
}

void Kunkun::KunkunMove(float move_speed, float rotate_angular_speed) {
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

void Kunkun::Basketball_turret_Rotate() {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    auto diff = input_data.mouse_cursor_position - position_;
    if (glm::length(diff) < 1e-4) {
      turret_rotation_ = rotation_;
    } else {
      turret_rotation_ = std::atan2(diff.y, diff.x) - glm::radians(90.0f);
    }
  }
}

void Kunkun::Basketball_Fire() {
  if (fire_count_down_ == 0) {
    auto player = game_core_->GetPlayer(player_id_);
    if (player) {
      auto &input_data = player->GetInputData();
      if (input_data.mouse_button_down[GLFW_MOUSE_BUTTON_LEFT]) {
        auto velocity = Rotate(glm::vec2{0.0f, 20.0f}, turret_rotation_);
        GenerateBullet<bullet::CannonBall>(
            position_ + Rotate({0.0f, 1.2f}, turret_rotation_),
            turret_rotation_, GetDamageScale(), velocity);
        fire_count_down_ = kTickPerSecond;  // Fire interval 1 second.
      }
    }
  }
  if (fire_count_down_) {
    fire_count_down_--;
  }
}

bool Kunkun::IsHit(glm::vec2 position) const {
  position = WorldToLocal(position);
  return position.x > -0.6f && position.x < 0.6f && position.y > -0.6f &&
         position.y < 0.6f &&
         position.x * position.x + position.y * position.y < 0.36f;
}

const char *Kunkun::UnitName() const {
  return "Zhiyinitaimei";
}

const char *Kunkun::Author() const {
  return "Jingyuan ma";
}
}  // namespace battle_game::unit