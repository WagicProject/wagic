#include "PrecompiledHeader.h"

#include "ObjectAnalytics.h"

#include "CardPrimitive.h"
#include "DebugRoutines.h"
#include "MTGCard.h"
#include "MTGCardInstance.h"

namespace ObjectAnalytics
{
    void DumpStatistics()
    {
#ifdef TRACK_OBJECT_USAGE

        DebugTrace("-----------------------------------------------------------");
        DebugTrace("Object Usage Stats" << std::endl);
        DebugTrace("CardPrimitive current count: " << InstanceCounter<CardPrimitive>::GetCurrentObjectCount());
        DebugTrace("CardPrimitive current byte usage: " << InstanceCounter<CardPrimitive>::GetCurrentByteCount());
        DebugTrace("CardPrimitive max count: " << InstanceCounter<CardPrimitive>::GetMaximumObjectCount());
        DebugTrace("CardPrimitive max byte usage: " << InstanceCounter<CardPrimitive>::GetMaximumByteCount() << std::endl);

        DebugTrace("MTGCard current count: " << InstanceCounter<MTGCard>::GetCurrentObjectCount());
        DebugTrace("MTGCard current byte usage: " << InstanceCounter<MTGCard>::GetCurrentByteCount());
        DebugTrace("MTGCard max count: " << InstanceCounter<MTGCard>::GetMaximumObjectCount());
        DebugTrace("MTGCard max byte usage: " << InstanceCounter<MTGCard>::GetMaximumByteCount() << std::endl);

        DebugTrace("MTGCardInstance current count: " << InstanceCounter<MTGCardInstance>::GetCurrentObjectCount());
        DebugTrace("MTGCardInstance current byte usage: " << InstanceCounter<MTGCardInstance>::GetCurrentByteCount());
        DebugTrace("MTGCardInstance max count: " << InstanceCounter<MTGCardInstance>::GetMaximumObjectCount());
        DebugTrace("MTGCardInstance max byte usage: " << InstanceCounter<MTGCardInstance>::GetMaximumByteCount() << std::endl);

        DebugTrace("-----------------------------------------------------------");

#endif
    }
}

