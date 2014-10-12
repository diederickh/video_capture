#include <videocapture/decklink/Decklink.h>

namespace ca {

  Decklink::Decklink() {

  }

  Decklink::~Decklink() {
  }

  int Decklink::listDevices() {

    std::vector<Device> devs = getDevices();

    return 0;
  }

  std::vector<Device> Decklink::getDevices() {
    
    std::vector<Device> devs;

    IDeckLinkIterator* iter = CreateDeckLinkIteratorInstance();
    if (NULL == iter) {
      printf("Error: failed to get a decklink iterator. Do you have decklink devices installed?\n");
      return devs;
    }

    int i = 0;
    IDeckLink* dl = NULL;
    while (S_OK == iter->Next(&dl)) {
      printf("Found a decklink device!\n");
    }
    printf("HMM..\n");
    
    return devs;
  }

} /* namespace ca */
