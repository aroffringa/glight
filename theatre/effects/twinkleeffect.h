#ifndef THEATRE_TWINKLE_EFFECT_H_
#define THEATRE_TWINKLE_EFFECT_H_

#include <random>

#include "../effect.h"
#include "../transition.h"

namespace glight::theatre {

class TwinkleEffect final : public Effect {
 public:
  TwinkleEffect() : Effect(1) {}

  EffectType GetType() const override { return EffectType::Twinkle; }

  void SetAverageDelay(double delay) { average_delay_ = delay; }
  double AverageDelay() const { return average_delay_; }

  void SetHoldTime(double hold) { hold_time_ = hold; }
  double HoldTime() const { return hold_time_; }

  void SetTransitionIn(const Transition& transition) {
    transition_in_ = transition;
  }
  const Transition& GetTransitionIn() const { return transition_in_; }

  void SetTransitionOut(const Transition& transition) {
    transition_out_ = transition;
  }
  const Transition& GetTransitionOut() const { return transition_out_; }

 protected:
  void MixImplementation(const ControlValue* values, const Timing& timing,
                         bool primary) override {
    if (values[0]) {
      if (previous_time_[primary] == -1.0)
        previous_time_[primary] = timing.TimeInMS();
      inputs_[primary].resize(Connections().size());
      for (size_t i = 0; i != Connections().size(); ++i) {
        MixInput(Connections()[i], inputs_[primary][i], values[0], timing,
                 primary);
      }
    }
    previous_time_[primary] = timing.TimeInMS();
  }

 private:
  enum class State { Waiting, TransitionIn, Hold, TransitionOut };
  struct InputData {
    State state = State::TransitionOut;
    double state_timer = 0.0;
  };

  void MixInput(const std::pair<Controllable*, size_t>& connection,
                InputData& input, const ControlValue& value,
                const Timing& timing, bool primary) {
    const double time_passed = timing.TimeInMS() - previous_time_[primary];
    input.state_timer -= time_passed;
    switch (input.state) {
      case State::Waiting:
        if (input.state_timer <= 0.0) {
          input.state = State::TransitionIn;
          input.state_timer = transition_in_.LengthInMs();
        }
        break;
      case State::TransitionIn:
        if (input.state_timer <= 0.0) {
          input.state_timer = hold_time_;
          input.state = State::Hold;
          connection.first->MixInput(connection.second, value);
        } else {
          const double transition_point =
              transition_out_.LengthInMs() - input.state_timer;
          const ControlValue transition_value =
              transition_in_.InValue(transition_point, timing);
          connection.first->MixInput(connection.second,
                                     transition_value * value);
        }
        break;
      case State::Hold:
        connection.first->MixInput(connection.second, value);
        if (input.state_timer <= 0.0) {
          input.state = State::TransitionOut;
          input.state_timer = transition_out_.LengthInMs();
        }
        break;
      case State::TransitionOut:
        if (input.state_timer <= 0.0) {
          std::exponential_distribution<double> distribution(1.0 /
                                                             average_delay_);
          input.state_timer = distribution(timing.RNG());
          input.state = State::Waiting;
        } else {
          const ControlValue transition_value =
              transition_out_.InValue(input.state_timer, timing);
          connection.first->MixInput(connection.second,
                                     transition_value * value);
        }
        break;
    }
  }

  double previous_time_[2] = {-1.0, -1.0};

  double average_delay_ = 1500.0;
  double hold_time_ = 0.0;
  Transition transition_in_ = Transition(300, TransitionType::Fade);
  Transition transition_out_ = Transition(300, TransitionType::Fade);
  std::vector<InputData> inputs_[2];
};

}  // namespace glight::theatre

#endif
