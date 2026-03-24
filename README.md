**C++ 23 Constexpr Header-only logging library (`ssplog.hpp`)**

Uses `logfmt` style: https://brandur.org/logfmt

To use, simply:
```C++
#include "ssplog.hpp"

using namespace ssp::log;

// Create logger instance
auto logger = Logger
{
  [](const std::string& message) {
    std::cout << message << std::endl;
  },
  {
    {"Category", "SSP"},
    {"Version",  1337}
  }
};

logger.info("Log Message" {{"Key1", "Field1"}, {"Key2", "Field2"}})
```

### Example Usage:

```C++
#include <memory>
#include <iostream>

#include <ssplog.hpp>

#include "engine.hpp"
#include "gui_top.hpp"

using namespace ssp::log;
using namespace ssp::engine;


auto logger = Logger
{
  [](const std::string& message) {
    std::cout << message << std::endl;
  },
  {
    {"Category", "SSP"},
    {"Version",  1337}
  }
};


int main(int argc, char* argv[]) {
  const auto log_level = log_level::info;

  logger.info("Setting Up: SSP Engine");

  auto engine = Engine();
  std::unique_ptr<dsp::SpectrogramProcessor> SpectraInstance(nullptr);

  // Check for component support
  if (engine.hasSpectrogramSupport())  {
    SpectraInstance.swap(engine.getSpectrogram());
    SpectraInstance->start();
    logger.info("SSP Engine Loaded",
      {"Module", "Spectrogram"}
    );
  } else {
    logger.warn("Skipping module",
      {"Module", "Spectrogram"}
    );
  }

  logger.info("SSP Engine: Ready");
  logger.info("Setting Up: SSP GUI");
  setup_imgui();

  logger.log(log_level, "SSP Ready");
  return 0;
}
```

```
time=2026-03-22T00:32:55Z level=info message="Setting Up: SSP Engine" Category="SSP" Version=1337
time=2026-03-22T00:32:55Z level=info message="SSP Engine Loaded" Category="SSP" Version=1337 Module="Spectrogram"
time=2026-03-22T00:32:55Z level=info message="SSP Engine: Ready" Category="SSP" Version=1337
time=2026-03-22T00:32:55Z level=info message="Setting Up: SSP GUI" Category="SSP" Version=1337
time=2026-03-22T00:33:01Z level=info message="SSP Ready" Category="SSP" Version=1337
```
