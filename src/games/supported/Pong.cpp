/* *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare and
 *   the Reinforcement Learning and Artificial Intelligence Laboratory
 * Released under the GNU General Public License; see License.txt for details.
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 */

#include "Pong.hpp"

#include "../RomUtils.hpp"

namespace ale {

PongSettings::PongSettings() { reset(); }

/* create a new instance of the rom */
RomSettings* PongSettings::clone() const {
  return new PongSettings(*this);
}

/* process the latest information from ALE */
void PongSettings::step(const System& system) {
  // update the reward
  int x = readRam(&system, 13); // cpu score
  int y = readRam(&system, 14); // player score
  reward_t score = y - x;
  m_reward_p1 = score - m_score;
  m_reward_p2 = -m_reward_p1;
  m_score = score;

  if((readRam(&system, 0x8c - 0x80) & 0xf0) == 0){
      // 0x8c describes the motion of the ball.
      // if it is 0, then it is going nowhere,
      // i.e. waiting to be served
      no_serve_counter++;
  }
  else{
      no_serve_counter = 0;
  }
  if(no_serve_counter >= 120) {
      // gives player 2 full seconds to serve, then
      // starts penalizing
      if (readRam(&system, 0x92 - 0x80) == 0){
          // is player 2's serve, penalize player 2
          m_reward_p2--;
      }
      else{
          // is player 1's serve, penalize player 1
          m_reward_p1--;
      }
      no_serve_counter = 0;
    }
  // update terminal status
  // (game over when a player reaches 21)
  m_terminal = x == 21 || y == 21;
}

/* is end of game */
bool PongSettings::isTerminal() const { return m_terminal; };

/* get the most recently observed reward */
reward_t PongSettings::getReward() const { return m_reward_p1; }
reward_t PongSettings::getRewardP2() const { return m_reward_p2; }
//P3 is on same team as P1, P2-P4
reward_t PongSettings::getRewardP3() const { return m_reward_p1; };
reward_t PongSettings::getRewardP4() const { return m_reward_p2; };


/* is an action part of the minimal set? */
bool PongSettings::isMinimal(const Action& a) const {
  switch (a) {
    case PLAYER_A_NOOP:
    case PLAYER_A_FIRE:
    case PLAYER_A_RIGHT:
    case PLAYER_A_LEFT:
    case PLAYER_A_RIGHTFIRE:
    case PLAYER_A_LEFTFIRE:
      return true;
    default:
      return false;
  }
}

/* reset the state of the game */
void PongSettings::reset() {
  m_reward_p1 = 0;
  m_reward_p2 = 0;
  m_score = 0;
  m_terminal = false;
  no_serve_counter = 0;
}

/* saves the state of the rom settings */
void PongSettings::saveState(Serializer& ser) {
  ser.putInt(m_reward_p1);
  ser.putInt(m_reward_p2);
  ser.putInt(m_score);
  ser.putBool(m_terminal);
  ser.putInt(no_serve_counter);
}

// loads the state of the rom settings
void PongSettings::loadState(Deserializer& ser) {
  m_reward_p1 = ser.getInt();
  m_reward_p2 = ser.getInt();
  m_score = ser.getInt();
  m_terminal = ser.getBool();
  no_serve_counter = ser.getInt();
}

// returns a list of mode that the game can be played in
ModeVect PongSettings::getAvailableModes() {
  return {1, 2};
}
ModeVect PongSettings::get2PlayerModes() {
  return {3, 4,
          9, 10,
          13,14,
          19,20,
          23,24,25,26,27,28,
          35,36,
          39,40,
          43,44,45,46};
}
ModeVect PongSettings::get4PlayerModes() {
  return {5,6,7,8,
          11,12,
          15,16,17,18,
          21,22,
          29,30,31,32,
          33,34,
          37,38,
          41,42,
          47,48,49,50};
}

// set the mode of the game
// the given mode must be one returned by the previous function
void PongSettings::setMode(
    game_mode_t m, System& system,
    std::unique_ptr<StellaEnvironmentWrapper> environment) {
  game_mode_t target = m - 1;

  // press select until the correct mode is reached
  while (readRam(&system, 0x96) != target) {
    environment->pressSelect(2);
  }
  //reset the environment to apply changes.
  environment->softReset();
}

// The left difficulty switch sets the width of the CPU opponent's bat.
// The right difficulty switch sets the width of the player's bat.
DifficultyVect PongSettings::getAvailableDifficulties() {
  return {0, 1, 2, 3};
}

}  // namespace ale
