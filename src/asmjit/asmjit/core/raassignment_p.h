// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_RAASSIGNMENT_P_H
#define _ASMJIT_CORE_RAASSIGNMENT_P_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_COMPILER

// [Dependencies]
#include "../core/radefs_p.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_ra
//! \{

// ============================================================================
// [asmjit::RAAssignment]
// ============================================================================

class RAAssignment {
  ASMJIT_NONCOPYABLE(RAAssignment)

public:
  enum Ids : uint32_t {
    kPhysNone = 0xFF,
    kWorkNone = RAWorkReg::kIdNone
  };

  enum DirtyBit : uint32_t {
    kClean = 0,
    kDirty = 1
  };

  // --------------------------------------------------------------------------
  // [Layout]
  // --------------------------------------------------------------------------

  struct Layout {
    inline void reset() noexcept {
      physIndex.reset();
      physCount.reset();
      physTotal = 0;
      workCount = 0;
      workRegs = nullptr;
    }

    RARegIndex physIndex;                //!< Index of architecture registers per group.
    RARegCount physCount;                //!< Count of architecture registers per group.
    uint32_t physTotal;                  //!< Count of physical registers of all groups.
    uint32_t workCount;                  //!< Count of work registers.
    const RAWorkRegs* workRegs;          //!< WorkRegs data (vector).
  };

  // --------------------------------------------------------------------------
  // [PhysToWorkMap]
  // --------------------------------------------------------------------------

  struct PhysToWorkMap {
    static inline size_t sizeOf(uint32_t count) noexcept {
      return sizeof(PhysToWorkMap) - sizeof(uint32_t) + size_t(count) * sizeof(uint32_t);
    }

    inline void reset(uint32_t count) noexcept {
      assigned.reset();
      dirty.reset();

      for (uint32_t i = 0; i < count; i++)
        workIds[i] = kWorkNone;
    }

    inline void copyFrom(const PhysToWorkMap* other, uint32_t count) noexcept {
      size_t size = sizeOf(count);
      std::memcpy(this, other, size);
    }

    RARegMask assigned;                  //!< Assigned registers (each bit represents one physical reg).
    RARegMask dirty;                     //!< Dirty registers (spill slot out of sync or no spill slot).
    uint32_t workIds[1 /* ... */];       //!< PhysReg to WorkReg mapping.
  };

  // --------------------------------------------------------------------------
  // [WorkToPhysMap]
  // --------------------------------------------------------------------------

  struct WorkToPhysMap {
    static inline size_t sizeOf(uint32_t count) noexcept {
      return size_t(count) * sizeof(uint8_t);
    }

    inline void reset(uint32_t count) noexcept {
      for (uint32_t i = 0; i < count; i++)
        physIds[i] = kPhysNone;
    }

    inline void copyFrom(const WorkToPhysMap* other, uint32_t count) noexcept {
      size_t size = sizeOf(count);
      if (ASMJIT_LIKELY(size))
        std::memcpy(this, other, size);
    }

    uint8_t physIds[1 /* ... */];        //!< WorkReg to PhysReg mapping
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline RAAssignment() noexcept {
    _layout.reset();
    resetMaps();
  }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  inline void initLayout(const RARegCount& physCount, const RAWorkRegs& workRegs) noexcept {
    // Layout must be initialized before data.
    ASMJIT_ASSERT(_physToWorkMap == nullptr);
    ASMJIT_ASSERT(_workToPhysMap == nullptr);

    _layout.physIndex.buildIndexes(physCount);
    _layout.physCount = physCount;
    _layout.physTotal = uint32_t(_layout.physIndex[BaseReg::kGroupVirt - 1]) +
                        uint32_t(_layout.physCount[BaseReg::kGroupVirt - 1]) ;
    _layout.workCount = workRegs.size();
    _layout.workRegs = &workRegs;
  }

  inline void initMaps(PhysToWorkMap* physToWorkMap, WorkToPhysMap* workToPhysMap) noexcept {
    _physToWorkMap = physToWorkMap;
    _workToPhysMap = workToPhysMap;
    for (uint32_t group = 0; group < BaseReg::kGroupVirt; group++)
      _physToWorkIds[group] = physToWorkMap->workIds + _layout.physIndex.get(group);
  }

  inline void resetMaps() noexcept {
    _physToWorkMap = nullptr;
    _workToPhysMap = nullptr;
    for (uint32_t group = 0; group < BaseReg::kGroupVirt; group++)
      _physToWorkIds[group] = nullptr;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline PhysToWorkMap* physToWorkMap() const noexcept { return _physToWorkMap; }
  inline WorkToPhysMap* workToPhysMap() const noexcept { return _workToPhysMap; }

  inline RARegMask& assigned() noexcept { return _physToWorkMap->assigned; }
  inline const RARegMask& assigned() const noexcept { return _physToWorkMap->assigned; }
  inline uint32_t assigned(uint32_t group) const noexcept { return _physToWorkMap->assigned[group]; }

  inline RARegMask& dirty() noexcept { return _physToWorkMap->dirty; }
  inline const RARegMask& dirty() const noexcept { return _physToWorkMap->dirty; }
  inline uint32_t dirty(uint32_t group) const noexcept { return _physToWorkMap->dirty[group]; }

  inline uint32_t workToPhysId(uint32_t group, uint32_t workId) const noexcept {
    ASMJIT_UNUSED(group);
    ASMJIT_ASSERT(workId != kWorkNone);
    ASMJIT_ASSERT(workId < _layout.workCount);
    return _workToPhysMap->physIds[workId];
  }

  inline uint32_t physToWorkId(uint32_t group, uint32_t physId) const noexcept {
    ASMJIT_ASSERT(physId < Globals::kMaxPhysRegs);
    return _physToWorkIds[group][physId];
  }

  inline bool isPhysAssigned(uint32_t group, uint32_t physId) const noexcept {
    ASMJIT_ASSERT(physId < Globals::kMaxPhysRegs);
    return Support::bitTest(_physToWorkMap->assigned[group], physId);
  }

  inline bool isPhysDirty(uint32_t group, uint32_t physId) const noexcept {
    ASMJIT_ASSERT(physId < Globals::kMaxPhysRegs);
    return Support::bitTest(_physToWorkMap->dirty[group], physId);
  }

  // --------------------------------------------------------------------------
  // [Assignment]
  // --------------------------------------------------------------------------

  // These are low-level allocation helpers that are used to update the current
  // mappings between physical and virt/work registers and also to update masks
  // that represent allocated and dirty registers. These functions don't emit
  // any code; they are only used to update and keep all mappings in sync.

  //! Assign [VirtReg/WorkReg] to a physical register.
  ASMJIT_INLINE void assign(uint32_t group, uint32_t workId, uint32_t physId, uint32_t dirty) noexcept {
    ASMJIT_ASSERT(workToPhysId(group, workId) == kPhysNone);
    ASMJIT_ASSERT(physToWorkId(group, physId) == kWorkNone);
    ASMJIT_ASSERT(!isPhysAssigned(group, physId));
    ASMJIT_ASSERT(!isPhysDirty(group, physId));

    _workToPhysMap->physIds[workId] = uint8_t(physId);
    _physToWorkIds[group][physId] = workId;

    uint32_t regMask = Support::mask(physId);
    _physToWorkMap->assigned[group] |= regMask;
    _physToWorkMap->dirty[group] |= regMask & Support::bitMaskFromBool<uint32_t>(dirty);

    verify();
  }

  //! Reassign [VirtReg/WorkReg] to `dstPhysId` from `srcPhysId`.
  ASMJIT_INLINE void reassign(uint32_t group, uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept {
    ASMJIT_ASSERT(dstPhysId != srcPhysId);
    ASMJIT_ASSERT(workToPhysId(group, workId) == srcPhysId);
    ASMJIT_ASSERT(physToWorkId(group, srcPhysId) == workId);
    ASMJIT_ASSERT(isPhysAssigned(group, srcPhysId) == true);
    ASMJIT_ASSERT(isPhysAssigned(group, dstPhysId) == false);

    _workToPhysMap->physIds[workId] = uint8_t(dstPhysId);
    _physToWorkIds[group][srcPhysId] = kWorkNone;
    _physToWorkIds[group][dstPhysId] = workId;

    uint32_t srcMask = Support::mask(srcPhysId);
    uint32_t dstMask = Support::mask(dstPhysId);

    uint32_t dirty = (_physToWorkMap->dirty[group] & srcMask) != 0;
    uint32_t regMask = dstMask | srcMask;

    _physToWorkMap->assigned[group] ^= regMask;
    _physToWorkMap->dirty[group] ^= regMask & Support::bitMaskFromBool<uint32_t>(dirty);

    verify();
  }

  ASMJIT_INLINE void swap(uint32_t group, uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept {
    ASMJIT_ASSERT(aPhysId != bPhysId);
    ASMJIT_ASSERT(workToPhysId(group, aWorkId) == aPhysId);
    ASMJIT_ASSERT(workToPhysId(group, bWorkId) == bPhysId);
    ASMJIT_ASSERT(physToWorkId(group, aPhysId) == aWorkId);
    ASMJIT_ASSERT(physToWorkId(group, bPhysId) == bWorkId);
    ASMJIT_ASSERT(isPhysAssigned(group, aPhysId));
    ASMJIT_ASSERT(isPhysAssigned(group, bPhysId));

    _workToPhysMap->physIds[aWorkId] = uint8_t(bPhysId);
    _workToPhysMap->physIds[bWorkId] = uint8_t(aPhysId);
    _physToWorkIds[group][aPhysId] = bWorkId;
    _physToWorkIds[group][bPhysId] = aWorkId;

    uint32_t aMask = Support::mask(aPhysId);
    uint32_t bMask = Support::mask(bPhysId);

    uint32_t flipMask = Support::bitMaskFromBool<uint32_t>(
      ((_physToWorkMap->dirty[group] & aMask) != 0) ^
      ((_physToWorkMap->dirty[group] & bMask) != 0));

    uint32_t regMask = aMask | bMask;
    _physToWorkMap->dirty[group] ^= regMask & flipMask;

    verify();
  }

  //! Unassign [VirtReg/WorkReg] from a physical register.
  ASMJIT_INLINE void unassign(uint32_t group, uint32_t workId, uint32_t physId) noexcept {
    ASMJIT_ASSERT(physId < Globals::kMaxPhysRegs);
    ASMJIT_ASSERT(workToPhysId(group, workId) == physId);
    ASMJIT_ASSERT(physToWorkId(group, physId) == workId);
    ASMJIT_ASSERT(isPhysAssigned(group, physId));

    _workToPhysMap->physIds[workId] = kPhysNone;
    _physToWorkIds[group][physId] = kWorkNone;

    uint32_t regMask = Support::mask(physId);
    _physToWorkMap->assigned[group] &= ~regMask;
    _physToWorkMap->dirty[group] &= ~regMask;

    verify();
  }

  inline void makeClean(uint32_t group, uint32_t workId, uint32_t physId) noexcept {
    ASMJIT_UNUSED(workId);

    uint32_t regMask = Support::mask(physId);
    _physToWorkMap->dirty[group] &= ~regMask;
  }

  inline void makeDirty(uint32_t group, uint32_t workId, uint32_t physId) noexcept {
    ASMJIT_UNUSED(workId);

    uint32_t regMask = Support::mask(physId);
    _physToWorkMap->dirty[group] |= regMask;
  }

  // --------------------------------------------------------------------------
  // [Copy / Swap]
  // --------------------------------------------------------------------------

  inline void copyFrom(const PhysToWorkMap* physToWorkMap, const WorkToPhysMap* workToPhysMap) noexcept {
    std::memcpy(_physToWorkMap, physToWorkMap, PhysToWorkMap::sizeOf(_layout.physTotal));
    std::memcpy(_workToPhysMap, workToPhysMap, WorkToPhysMap::sizeOf(_layout.workCount));
  }

  inline void copyFrom(const RAAssignment& other) noexcept {
    copyFrom(other.physToWorkMap(), other.workToPhysMap());
  }

  inline void swapWith(RAAssignment& other) noexcept {
    std::swap(_workToPhysMap, other._workToPhysMap);
    std::swap(_physToWorkMap, other._physToWorkMap);

    for (uint32_t group = 0; group < BaseReg::kGroupVirt; group++)
      std::swap(_physToWorkIds[group], other._physToWorkIds[group]);
  }

  // --------------------------------------------------------------------------
  // [Equals]
  // --------------------------------------------------------------------------

  // Not really useful outside of debugging.
  bool equals(const RAAssignment& other) const noexcept {
    // Layout should always match.
    if (_layout.physIndex != other._layout.physIndex ||
        _layout.physCount != other._layout.physCount ||
        _layout.physTotal != other._layout.physTotal ||
        _layout.workCount != other._layout.workCount ||
        _layout.workRegs  != other._layout.workRegs)
      return false;

    uint32_t physTotal = _layout.physTotal;
    uint32_t workCount = _layout.workCount;

    for (uint32_t physId = 0; physId < physTotal; physId++) {
      uint32_t thisWorkId = _physToWorkMap->workIds[physId];
      uint32_t otherWorkId = other._physToWorkMap->workIds[physId];
      if (thisWorkId != otherWorkId)
        return false;
    }

    for (uint32_t workId = 0; workId < workCount; workId++) {
      uint32_t thisPhysId = _workToPhysMap->physIds[workId];
      uint32_t otherPhysId = other._workToPhysMap->physIds[workId];
      if (thisPhysId != otherPhysId)
        return false;
    }

    if (_physToWorkMap->assigned != other._physToWorkMap->assigned ||
        _physToWorkMap->dirty    != other._physToWorkMap->dirty    )
      return false;

    return true;
  }

  // --------------------------------------------------------------------------
  // [Verify]
  // --------------------------------------------------------------------------

#if defined(ASMJIT_BUILD_DEBUG)
  ASMJIT_NOINLINE void verify() noexcept {
    // Verify WorkToPhysMap.
    {
      for (uint32_t workId = 0; workId < _layout.workCount; workId++) {
        uint32_t physId = _workToPhysMap->physIds[workId];
        if (physId != kPhysNone) {
          const RAWorkReg* workReg = _layout.workRegs->at(workId);
          uint32_t group = workReg->group();
          ASMJIT_ASSERT(_physToWorkIds[group][physId] == workId);
        }
      }
    }

    // Verify PhysToWorkMap.
    {
      for (uint32_t group = 0; group < BaseReg::kGroupVirt; group++) {
        uint32_t physCount = _layout.physCount[group];
        for (uint32_t physId = 0; physId < physCount; physId++) {
          uint32_t workId = _physToWorkIds[group][physId];
          if (workId != kWorkNone) {
            ASMJIT_ASSERT(_workToPhysMap->physIds[workId] == physId);
          }
        }
      }
    }
  }
#else
  inline void verify() noexcept {}
#endif

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Layout _layout;                        //!< Physical registers layout.
  WorkToPhysMap* _workToPhysMap;         //!< WorkReg to PhysReg mapping.
  PhysToWorkMap* _physToWorkMap;         //!< PhysReg to WorkReg mapping and assigned/dirty bits.
  uint32_t* _physToWorkIds[BaseReg::kGroupVirt]; //!< Optimization to translate PhysRegs to WorkRegs faster.
};
//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_CORE_RAASSIGNMENT_P_H
