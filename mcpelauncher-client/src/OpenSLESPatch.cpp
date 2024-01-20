#include "OpenSLESPatch.h"
#include <SLES/OpenSLES.h>
#include <hybris/hook.h>

// TODO Implement audio

void OpenSLESPatch::install() {
    static SLInterfaceID_ SL_IID_ENGINE { .time_low = 0, .time_mid = 0, .time_hi_and_version = 0, .clock_seq = 0, .node = { 1, 2, 3, 4, 5, 6 }  };
    static SLInterfaceID_ SL_IID_BUFFERQUEUE { .time_low = 0, .time_mid = 0, .time_hi_and_version = 0, .clock_seq = 0, .node = { 2, 3, 4, 5, 6, 7 }  };
    static SLInterfaceID_ SL_IID_VOLUME { .time_low = 0, .time_mid = 0, .time_hi_and_version = 0, .clock_seq = 0, .node = { 1, 2, 3, 4, 5, 8 }  };
    static SLInterfaceID_ SL_IID_PLAY { .time_low = 0, .time_mid = 0, .time_hi_and_version = 0, .clock_seq = 0, .node = { 1, 2, 3, 4, 5, 9 }  };
    hybris_hook("SL_IID_ENGINE", (void*)&SL_IID_ENGINE);
    hybris_hook("SL_IID_BUFFERQUEUE", (void*)&SL_IID_BUFFERQUEUE);
    hybris_hook("SL_IID_VOLUME", (void*)&SL_IID_VOLUME);
    hybris_hook("SL_IID_PLAY", (void*)&SL_IID_PLAY);
    hybris_hook("slCreateEngine", (void*) + [](SLObjectItf *pEngine, SLuint32 numOptions, const SLEngineOption *pEngineOptions, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired) -> SLresult {
      // auto& engine = *(SLObjectItf_**)pEngine;
      // *engine
      *pEngine = new SLObjectItf_ *(new SLObjectItf_({
        .Realize = [](SLObjectItf self, SLboolean async) -> SLresult {
          return 0;
        },
        .Resume = [](SLObjectItf self, SLboolean async) -> SLresult {
          return 0;
        },
        .GetState = [](SLObjectItf self,
		SLuint32 * pState) -> SLresult {
          return 0;
        },
        .GetInterface = [](SLObjectItf self,
		const SLInterfaceID iid,
		void * pInterface) -> SLresult {
          static SLEngineItf_ engine = {
            .CreateLEDDevice = [](SLEngineItf self,
          SLObjectItf * pDevice,
          SLuint32 deviceID,
          SLuint32 numInterfaces,
          const SLInterfaceID * pInterfaceIds,
          const SLboolean * pInterfaceRequired) -> SLresult {
            return 0;
          },
          .CreateVibraDevice = [](SLEngineItf self,
          SLObjectItf * pDevice,
          SLuint32 deviceID,
          SLuint32 numInterfaces,
          const SLInterfaceID * pInterfaceIds,
          const SLboolean * pInterfaceRequired) -> SLresult {
            return 0;
          },
          .CreateAudioPlayer = [](SLEngineItf self,
		SLObjectItf * pPlayer,
		SLDataSource *pAudioSrc,
		SLDataSink *pAudioSnk,
		SLuint32 numInterfaces,
		const SLInterfaceID * pInterfaceIds,
		const SLboolean * pInterfaceRequired) -> SLresult {
              *pPlayer = new SLObjectItf_ *(new SLObjectItf_({
                  .Realize = [](SLObjectItf self, SLboolean async) -> SLresult {
                    return 0;
                  },
                  .Resume = [](SLObjectItf self, SLboolean async) -> SLresult {
                    return 0;
                  },
                  .GetState = [](SLObjectItf self,
              SLuint32 * pState) -> SLresult {
                    return 0;
                  },
                  .GetInterface = [](SLObjectItf self,
              const SLInterfaceID iid,
              void * pInterface) -> SLresult {
                    static SLEngineItf_ engine = {
                      .CreateLEDDevice = [](SLEngineItf self,
                    SLObjectItf * pDevice,
                    SLuint32 deviceID,
                    SLuint32 numInterfaces,
                    const SLInterfaceID * pInterfaceIds,
                    const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },
                    .CreateVibraDevice = [](SLEngineItf self,
                    SLObjectItf * pDevice,
                    SLuint32 deviceID,
                    SLuint32 numInterfaces,
                    const SLInterfaceID * pInterfaceIds,
                    const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },
                    .CreateAudioPlayer = [](SLEngineItf self,
              SLObjectItf * pPlayer,
              SLDataSource *pAudioSrc,
              SLDataSink *pAudioSnk,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },
                    .CreateAudioRecorder = [](SLEngineItf self,
              SLObjectItf * pPlayer,
              SLDataSource *pAudioSrc,
              SLDataSink *pAudioSnk,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.CreateMidiPlayer = [](SLEngineItf self,
              SLObjectItf * pPlayer,
              SLDataSource *pMIDISrc,
              SLDataSource *pBankSrc,
              SLDataSink *pAudioOutput,
              SLDataSink *pVibra,
              SLDataSink *pLEDArray,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.CreateListener = [](SLEngineItf self,
              SLObjectItf * pListener,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.Create3DGroup = [](SLEngineItf self,
              SLObjectItf * pGroup,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.CreateOutputMix = [](SLEngineItf self,
              SLObjectItf * pMix,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.CreateMetadataExtractor = [](SLEngineItf self,
              SLObjectItf * pMetadataExtractor,
              SLDataSource * pDataSource,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.CreateExtensionObject = [](SLEngineItf self,
                  SLObjectItf * pObject,
                  void * pParameters,
                  SLuint32 objectID,
                  SLuint32 numInterfaces,
                  const SLInterfaceID * pInterfaceIds,
                  const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.QueryNumSupportedInterfaces = [](SLEngineItf self,
              SLuint32 objectID,
              SLuint32 * pNumSupportedInterfaces) -> SLresult {
                      return 0;
                    },.QuerySupportedInterfaces = [](SLEngineItf self,
              SLuint32 objectID,
              SLuint32 index,
              SLInterfaceID * pInterfaceId) -> SLresult {
                      return 0;
                    },.QueryNumSupportedExtensions = [](SLEngineItf self,
                  SLuint32 * pNumExtensions) -> SLresult {
                      return 0;
                    },.QuerySupportedExtension = [](SLEngineItf self,
                  SLuint32 index,
                  SLchar * pExtensionName,
                  SLint16 * pNameLength) -> SLresult {
                      return 0;
                    },.IsExtensionSupported = [](SLEngineItf self,
                  const SLchar * pExtensionName,
                  SLboolean * pSupported) -> SLresult {
                      return 0;
                    }
                    };
                    *((SLEngineItf_***)pInterface) = new SLEngineItf_*(&engine);
                    return 0;
                  },
                  .RegisterCallback = [](SLObjectItf self,
              slObjectCallback callback,
              void * pContext) -> SLresult {
                    return 0;
                  },
                  .AbortAsyncOperation = [](SLObjectItf self) -> void {

                  },
                  .Destroy = [](SLObjectItf self) -> void {

                  },
                  .SetPriority = [](SLObjectItf self,
              SLint32 priority,
              SLboolean preemptable) -> SLresult {
                    return 0;
                  },
                  .GetPriority = [](SLObjectItf self,
              SLint32 *pPriority,
              SLboolean *pPreemptable) -> SLresult {
                    return 0;
                  },
                  .SetLossOfControlInterfaces = [](SLObjectItf self,
              SLint16 numInterfaces,
              SLInterfaceID * pInterfaceIDs,
              SLboolean enabled) -> SLresult {
                    return 0;
                  }
                }));
            return 0;
          },
          .CreateAudioRecorder = [](SLEngineItf self,
		SLObjectItf * pPlayer,
		SLDataSource *pAudioSrc,
		SLDataSink *pAudioSnk,
		SLuint32 numInterfaces,
		const SLInterfaceID * pInterfaceIds,
		const SLboolean * pInterfaceRequired) -> SLresult {
            return 0;
          },.CreateMidiPlayer = [](SLEngineItf self,
		SLObjectItf * pPlayer,
		SLDataSource *pMIDISrc,
		SLDataSource *pBankSrc,
		SLDataSink *pAudioOutput,
		SLDataSink *pVibra,
		SLDataSink *pLEDArray,
		SLuint32 numInterfaces,
		const SLInterfaceID * pInterfaceIds,
		const SLboolean * pInterfaceRequired) -> SLresult {
            return 0;
          },.CreateListener = [](SLEngineItf self,
		SLObjectItf * pListener,
		SLuint32 numInterfaces,
		const SLInterfaceID * pInterfaceIds,
		const SLboolean * pInterfaceRequired) -> SLresult {
            return 0;
          },.Create3DGroup = [](SLEngineItf self,
		SLObjectItf * pGroup,
		SLuint32 numInterfaces,
		const SLInterfaceID * pInterfaceIds,
		const SLboolean * pInterfaceRequired) -> SLresult {
            return 0;
          },.CreateOutputMix = [](SLEngineItf self,
		SLObjectItf * pMix,
		SLuint32 numInterfaces,
		const SLInterfaceID * pInterfaceIds,
		const SLboolean * pInterfaceRequired) -> SLresult {
                *pMix = new SLObjectItf_ *(new SLObjectItf_({
                  .Realize = [](SLObjectItf self, SLboolean async) -> SLresult {
                    return 0;
                  },
                  .Resume = [](SLObjectItf self, SLboolean async) -> SLresult {
                    return 0;
                  },
                  .GetState = [](SLObjectItf self,
              SLuint32 * pState) -> SLresult {
                    return 0;
                  },
                  .GetInterface = [](SLObjectItf self,
              const SLInterfaceID iid,
              void * pInterface) -> SLresult {
                    static SLEngineItf_ engine = {
                      .CreateLEDDevice = [](SLEngineItf self,
                    SLObjectItf * pDevice,
                    SLuint32 deviceID,
                    SLuint32 numInterfaces,
                    const SLInterfaceID * pInterfaceIds,
                    const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },
                    .CreateVibraDevice = [](SLEngineItf self,
                    SLObjectItf * pDevice,
                    SLuint32 deviceID,
                    SLuint32 numInterfaces,
                    const SLInterfaceID * pInterfaceIds,
                    const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },
                    .CreateAudioPlayer = [](SLEngineItf self,
              SLObjectItf * pPlayer,
              SLDataSource *pAudioSrc,
              SLDataSink *pAudioSnk,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },
                    .CreateAudioRecorder = [](SLEngineItf self,
              SLObjectItf * pPlayer,
              SLDataSource *pAudioSrc,
              SLDataSink *pAudioSnk,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.CreateMidiPlayer = [](SLEngineItf self,
              SLObjectItf * pPlayer,
              SLDataSource *pMIDISrc,
              SLDataSource *pBankSrc,
              SLDataSink *pAudioOutput,
              SLDataSink *pVibra,
              SLDataSink *pLEDArray,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.CreateListener = [](SLEngineItf self,
              SLObjectItf * pListener,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.Create3DGroup = [](SLEngineItf self,
              SLObjectItf * pGroup,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.CreateOutputMix = [](SLEngineItf self,
              SLObjectItf * pMix,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.CreateMetadataExtractor = [](SLEngineItf self,
              SLObjectItf * pMetadataExtractor,
              SLDataSource * pDataSource,
              SLuint32 numInterfaces,
              const SLInterfaceID * pInterfaceIds,
              const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.CreateExtensionObject = [](SLEngineItf self,
                  SLObjectItf * pObject,
                  void * pParameters,
                  SLuint32 objectID,
                  SLuint32 numInterfaces,
                  const SLInterfaceID * pInterfaceIds,
                  const SLboolean * pInterfaceRequired) -> SLresult {
                      return 0;
                    },.QueryNumSupportedInterfaces = [](SLEngineItf self,
              SLuint32 objectID,
              SLuint32 * pNumSupportedInterfaces) -> SLresult {
                      return 0;
                    },.QuerySupportedInterfaces = [](SLEngineItf self,
              SLuint32 objectID,
              SLuint32 index,
              SLInterfaceID * pInterfaceId) -> SLresult {
                      return 0;
                    },.QueryNumSupportedExtensions = [](SLEngineItf self,
                  SLuint32 * pNumExtensions) -> SLresult {
                      return 0;
                    },.QuerySupportedExtension = [](SLEngineItf self,
                  SLuint32 index,
                  SLchar * pExtensionName,
                  SLint16 * pNameLength) -> SLresult {
                      return 0;
                    },.IsExtensionSupported = [](SLEngineItf self,
                  const SLchar * pExtensionName,
                  SLboolean * pSupported) -> SLresult {
                      return 0;
                    }
                    };
                    *((SLEngineItf_***)pInterface) = new SLEngineItf_*(&engine);
                    return 0;
                  },
                  .RegisterCallback = [](SLObjectItf self,
              slObjectCallback callback,
              void * pContext) -> SLresult {
                    return 0;
                  },
                  .AbortAsyncOperation = [](SLObjectItf self) -> void {

                  },
                  .Destroy = [](SLObjectItf self) -> void {

                  },
                  .SetPriority = [](SLObjectItf self,
              SLint32 priority,
              SLboolean preemptable) -> SLresult {
                    return 0;
                  },
                  .GetPriority = [](SLObjectItf self,
              SLint32 *pPriority,
              SLboolean *pPreemptable) -> SLresult {
                    return 0;
                  },
                  .SetLossOfControlInterfaces = [](SLObjectItf self,
              SLint16 numInterfaces,
              SLInterfaceID * pInterfaceIDs,
              SLboolean enabled) -> SLresult {
                    return 0;
                  }
                }));
            return 0;
          },.CreateMetadataExtractor = [](SLEngineItf self,
		SLObjectItf * pMetadataExtractor,
		SLDataSource * pDataSource,
		SLuint32 numInterfaces,
		const SLInterfaceID * pInterfaceIds,
		const SLboolean * pInterfaceRequired) -> SLresult {
            return 0;
          },.CreateExtensionObject = [](SLEngineItf self,
        SLObjectItf * pObject,
        void * pParameters,
        SLuint32 objectID,
        SLuint32 numInterfaces,
        const SLInterfaceID * pInterfaceIds,
        const SLboolean * pInterfaceRequired) -> SLresult {
            return 0;
          },.QueryNumSupportedInterfaces = [](SLEngineItf self,
		SLuint32 objectID,
		SLuint32 * pNumSupportedInterfaces) -> SLresult {
            return 0;
          },.QuerySupportedInterfaces = [](SLEngineItf self,
		SLuint32 objectID,
		SLuint32 index,
		SLInterfaceID * pInterfaceId) -> SLresult {
            return 0;
          },.QueryNumSupportedExtensions = [](SLEngineItf self,
        SLuint32 * pNumExtensions) -> SLresult {
            return 0;
          },.QuerySupportedExtension = [](SLEngineItf self,
        SLuint32 index,
        SLchar * pExtensionName,
        SLint16 * pNameLength) -> SLresult {
            return 0;
          },.IsExtensionSupported = [](SLEngineItf self,
        const SLchar * pExtensionName,
        SLboolean * pSupported) -> SLresult {
            return 0;
          }
          };
          *((SLEngineItf_***)pInterface) = new SLEngineItf_*(&engine);
          return 0;
        },
        .RegisterCallback = [](SLObjectItf self,
		slObjectCallback callback,
		void * pContext) -> SLresult {
          return 0;
        },
        .AbortAsyncOperation = [](SLObjectItf self) -> void {

        },
        .Destroy = [](SLObjectItf self) -> void {

        },
        .SetPriority = [](SLObjectItf self,
		SLint32 priority,
		SLboolean preemptable) -> SLresult {
          return 0;
        },
        .GetPriority = [](SLObjectItf self,
		SLint32 *pPriority,
		SLboolean *pPreemptable) -> SLresult {
          return 0;
        },
        .SetLossOfControlInterfaces = [](SLObjectItf self,
		SLint16 numInterfaces,
		SLInterfaceID * pInterfaceIDs,
		SLboolean enabled) -> SLresult {
          return 0;
        }
      }));
      return 0;
    });
}