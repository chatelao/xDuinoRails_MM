#ifndef CV_MANAGER_MOCK_H
#define CV_MANAGER_MOCK_H

#include "CvManager.h"
#include <map>

class CvManagerMock : public CvManager {
public:
  CvManagerMock() {}
  void    setup() override {}
  uint8_t getCv(int cv) override {
    if (cvs.count(cv)) {
      return cvs[cv];
    }
    return 0;
  }
  void setCv(int cv, uint8_t value) override { cvs[cv] = value; }

private:
  std::map<int, uint8_t> cvs;
};

#endif // CV_MANAGER_MOCK_H
