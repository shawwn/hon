SRC_DIR=SourceCode
LIBSPLINE_SRC_DIR=$(SRC_DIR)/LibSpline_Source
LIBRANDOM_SRC_DIR=$(SRC_DIR)/LibRandom_Source
LIBGLOBALS_SRC_DIR=$(SRC_DIR)/LibGlobals_Source
LIBFILENAME_SRC_DIR=$(SRC_DIR)/LibFilename_Source
LIBVECTOR_SRC_DIR=$(SRC_DIR)/LibVector_Source

# Source files
SRC= \
$(SRC_DIR)/BillboardLeaf.cpp \
$(SRC_DIR)/Branch.cpp \
$(SRC_DIR)/BranchInfo.cpp \
$(SRC_DIR)/Camera.cpp \
$(SRC_DIR)/FileAccess.cpp \
$(SRC_DIR)/FrondEngine.cpp \
$(SRC_DIR)/IndexedGeometry.cpp \
$(SRC_DIR)/LeafGeometry.cpp \
$(SRC_DIR)/LeafInfo.cpp \
$(SRC_DIR)/LeafLod.cpp \
$(SRC_DIR)/LightingEngine.cpp \
$(SRC_DIR)/ProjectedShadow.cpp \
$(SRC_DIR)/SimpleBillboard.cpp \
$(SRC_DIR)/SpeedTreeRT.cpp \
$(SRC_DIR)/TreeEngine.cpp \
$(SRC_DIR)/TreeInfo.cpp \
$(SRC_DIR)/WindEngine.cpp \
$(SRC_DIR)/WindInfo.cpp

LIBRANDOM_SRC= \
$(LIBRANDOM_SRC_DIR)/ExtendedReal.cpp \
$(LIBRANDOM_SRC_DIR)/IdvRandom.cpp \
$(LIBRANDOM_SRC_DIR)/RobertDavies_Random.cpp

LIBSPLINE_SRC= \
$(LIBSPLINE_SRC_DIR)/IdvSpline.cpp

LIBVECTOR_SRC= \
$(LIBVECTOR_SRC_DIR)/Region.cpp \
$(LIBVECTOR_SRC_DIR)/RotTransform.cpp \
$(LIBVECTOR_SRC_DIR)/Transform.cpp \
$(LIBVECTOR_SRC_DIR)/Vec.cpp \
$(LIBVECTOR_SRC_DIR)/Vec3.cpp

# Object directories
LIBSPLINE_OBJ_DIR=$(OBJ_DIR)/LibSpline
LIBRANDOM_OBJ_DIR=$(OBJ_DIR)/LibRandom
LIBVECTOR_OBJ_DIR=$(OBJ_DIR)/LibVector

# Object files
OBJS=$(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRC))
LIBSPLINE_OBJS=$(patsubst $(LIBSPLINE_SRC_DIR)%.cpp,$(LIBSPLINE_OBJ_DIR)%.o,$(LIBSPLINE_SRC))
LIBRANDOM_OBJS=$(patsubst $(LIBRANDOM_SRC_DIR)%.cpp,$(LIBRANDOM_OBJ_DIR)%.o,$(LIBRANDOM_SRC))
LIBVECTOR_OBJS=$(patsubst $(LIBVECTOR_SRC_DIR)%.cpp,$(LIBVECTOR_OBJ_DIR)%.o,$(LIBVECTOR_SRC))

ALL_OBJS=$(OBJS) $(LIBSPLINE_OBJS) $(LIBRANDOM_OBJS) $(LIBVECTOR_OBJS)

# Directory for library
LIBRARY_NAME=$(LIB_DIR)/libSpeedTreeRT.a

LIBRARY_NAME_DEBUG=$(LIB_DIR)/libSpeedTreeRT_debug.a

default:
	@echo "Call using:"
	@echo "OBJ_DIR=\"/path/to/put/files.o\" LIB_DIR=\"/path/to/put/lib.a\" make SpeedTreeRT[_debug]"

SpeedTreeRT: $(OBJ_DIR) $(LIBSPLINE_OBJ_DIR) $(LIBRANDOM_OBJ_DIR) $(LIBVECTOR_OBJ_DIR) $(LIBRARY_NAME)
SpeedTreeRT_debug: $(OBJ_DIR) $(LIBSPLINE_OBJ_DIR) $(LIBRANDOM_OBJ_DIR) $(LIBVECTOR_OBJ_DIR) $(LIBRARY_NAME_DEBUG)

$(LIBRARY_NAME): $(ALL_OBJS)
	$(AR) rcs $@ $(ALL_OBJS)

$(LIBRARY_NAME_DEBUG): $(ALL_OBJS)
	$(AR) rcs $@ $(ALL_OBJS)

$(OBJ_DIR):
	@if test ! -d $(OBJ_DIR); then mkdir -p $(OBJ_DIR); fi
	
$(LIBSPLINE_OBJ_DIR):	
	@if test ! -d $(LIBSPLINE_OBJ_DIR); then mkdir -p $(LIBSPLINE_OBJ_DIR); fi
	
$(LIBRANDOM_OBJ_DIR):
	@if test ! -d $(LIBRANDOM_OBJ_DIR); then mkdir -p $(LIBRANDOM_OBJ_DIR); fi
	
$(LIBVECTOR_OBJ_DIR):
	@if test ! -d $(LIBVECTOR_OBJ_DIR); then mkdir -p $(LIBVECTOR_OBJ_DIR); fi

$(LIB_DIR):
	@if test ! -d $(LIB_DIR); then mkdir -p $(LIB_DIR); fi

# Targets for all the objects follow

$(OBJ_DIR)/BillboardLeaf.o: $(SRC_DIR)/BillboardLeaf.cpp $(DEBUG_H) $(BILLBOARDLEAF_H) $(FILEACCESS_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/Branch.o: $(SRC_DIR)/Branch.cpp $(DEBUG_H) $(TREEENGINE_H) $(FILEACCESS_H) $(IDVRANDOM_H) $(BILLBOARD_LEAF_H) $(SPEEDTREERT_H) $(INDEXEDGEOMETRY_H) $(LIGHTINGENGINE_H) $(FRONDENGINE_H) $(RANDOM_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/BranchInfo.o: $(SRC_DIR)/BranchInfo.cpp $(DEBUG_H) $(TREEENGINE_H) $(FILEACCESS_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/Camera.o: $(SRC_DIR)/Camera.cpp $(DEBUG_H) $(TREEENGINE_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/FileAccess.o: $(SRC_DIR)/FileAccess.cpp $(DEBUG_H) $(FILEACCESS_H) $(ENDIAN_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/FrondEngine.o: $(SRC_DIR)/FrondEngine.cpp $(DEBUG_H) $(FRONDENGINE_H) $(INDEXEDGEOMETRY_H) $(LIGHTINGENGINE_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/IndexedGeometry.o: $(SRC_DIR)/IndexedGeometry.cpp $(DEBUG_H) $(WINDENGINE_H) $(INDEXEDGEOMETRY_H) $(UPVECTOR_H) $(ENDIAN_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/LeafGeometry.o: $(SRC_DIR)/LeafGeometry.cpp $(DEBUG_H) $(SPEEDTREERT_H) $(BILLBOARDLEAF_H) $(LEAFGEOMETRY_H) $(WINDENGINE_H) $(UPVECTOR_H) $(ENDIAN_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/LeafInfo.o: $(SRC_DIR)/LeafInfo.cpp $(DEBUG_H) $(TREEENGINE_H) $(FILEACCESS_H) $(IDVRANDOM_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/LeafLod.o: $(SRC_DIR)/LeafLod.cpp $(DEBUG_H) $(LEAFLOD_H) $(BILLBOARD_LEAF_H) $(IDVRANDOM_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/LightingEngine.o: $(SRC_DIR)/LightingEngine.cpp $(DEBUG_H) $(LIGHTINGENGINE_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/ProjectedShadow.o: $(SRC_DIR)/ProjectedShadow.cpp $(DEBUG_H) $(PROJECTED_SHADOW_H) $(IDVGLOBALS_H) $(IDVFILENAME_H) $(INDEXEDGEOMETRY_H) $(UPVECTOR_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/SimpleBillboard.o: $(SRC_DIR)/SimpleBillboard.cpp $(DEBUG_H) $(SIMPLEBILLBOARD_H) $(UPVECTOR_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/SpeedTreeRT.o: $(SRC_DIR)/SpeedTreeRT.cpp $(DEBUG_H) $(IDVGLOBALS_H) $(SPEEDTREERT_H) $(INDEXEDGEOMETRY_H) $(INSTANCES_H) $(TREEENGINE_H) $(WINDENGINE_H) $(SIMPLEBILLBOARD_H) $(LIGHTINGENGINE_H) $(LEAFGEOMETRY_H) $(FRONDENGINE_H) $(UPVECTOR_H) $(PROJECTEDSHADOW_H) $(FILEACCESS) $(EVALCODE_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/TreeEngine.o: $(SRC_DIR)/TreeEngine.cpp $(DEBUG_H) $(TREEENGINE_H) $(BILLBOARD_H) $(WINDENGINE_H) $(LEAFLOD_H) $(INDEXEDGEOMETRY_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/TreeInfo.o: $(SRC_DIR)/TreeInfo.cpp $(DEBUG_H) $(TREEENGINE_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/WindEngine.o: $(SRC_DIR)/WindEngine.cpp $(DEBUG_H) $(SPEEDTREERT_H) $(WINDENGINE_H) $(UPVECTOR_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/WindInfo.o: $(SRC_DIR)/WindInfo.cpp $(DEBUG_H) $(TREEENGINE_H) $(FILEACCESS_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# LibRandom

$(LIBRANDOM_OBJ_DIR)/ExtendedReal.o: $(LIBRANDOM_SRC_DIR)/ExtendedReal.cpp $(EXTENDEDREAL_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBRANDOM_OBJ_DIR)/IdvRandom.o: $(LIBRANDOM_SRC_DIR)/IdvRandom.cpp $(IDVRANDOM_H) $(ROBERTDAVIES_RANDOM_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBRANDOM_OBJ_DIR)/RobertDavies_Random.o: $(LIBRANDOM_SRC_DIR)/RobertDavies_Random.cpp $(ROBERTDAVIES_RANDOM)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# LibSpline

$(LIBSPLINE_OBJ_DIR)/IdvSpline.o: $(LIBSPLINE_SRC_DIR)/IdvSpline.cpp $(DEBUG_H) $(IDVSPLINE_H) $(IDVRANDOM_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# LibVector

$(LIBVECTOR_OBJ_DIR)/Region.o: $(LIBVECTOR_SRC_DIR)/Region.cpp $(IDVVECTOR_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBVECTOR_OBJ_DIR)/RotTransform.o: $(LIBVECTOR_SRC_DIR)/RotTransform.cpp $(IDVVECTOR_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBVECTOR_OBJ_DIR)/Transform.o: $(LIBVECTOR_SRC_DIR)/Transform.cpp $(IDVVECTOR_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBVECTOR_OBJ_DIR)/Vec.o: $(LIBVECTOR_SRC_DIR)/Vec.cpp $(IDVVECTOR_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBVECTOR_OBJ_DIR)/Vec3.o: $(LIBVECTOR_SRC_DIR)/Vec3.cpp $(IDVVECTOR_H)
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Headers and their dependencies

BILLBOARD_LEAF=$(SRC_DIR)/BillboardLeaf.h $(TREEENGINE_H)
$(SRC_DIR)/BillboardLeaf.h:

DEBUG_H=$(SRC_DIR)/Debug.h
$(SRC_DIR)/Debug.h:

ENDIAN_H=$(SRC_DIR)/Endian.h $(SPEEDTREERT_H)
$(SRC_DIR)/Endian.h:

EVALCODE_H=$(SRC_DIR)/EvalCode.h
$(SRC_DIR)/EvalCode.h:

FILEACCESS_H=$(SRC_DIR)/FileAccess.h $(IDVSPLINE_H)
$(SRC_DIR)/FileAccess.h:

FRONDENGINE_H=$(SRC_DIR)/FrondEngine.h $(SPEEDTREERT_H) $(IDVGLOBALS_H) $(IDVFILENAME_H) $(FILEACCESS_H)
$(SRC_DIR)/FrondEngine.h:

INDEXEDGEOMETRY_H=$(SRC_DIR)/IndexedGeometry.h $(DEBUG_H) $(SPEEDTREERT_H) $(IDVVECTOR_H)
$(SRC_DIR)/IndexedGeometry.h:

INSTANCES_H=$(SRC_DIR)/Instances.h $(IDVGLOBALS_H) $(IDVVECTOR_H)
$(SRC_DIR)/Instances.h:

LEAFGEOMETRY_H=$(SRC_DIR)/LeafGeometry.h $(DEBUG_H) $(SPEEDTREE_H)
$(SRC_DIR)/LeafGeometry.h:

LEAFLOD_H=$(SRC_DIR)/LeafLod.h $(TREEENGINE_H) $(STRUCTSINFO_H)
$(SRC_DIR)/LeafLod.h:

LIGHTING_ENGINE_H=$(SRC_DIR)/LightingEngine.h $(IDVGLOBALS_H) $(SPEEDTREERT_H) $(BILLBOARDLEAF_H) $(DEBUG_H)
$(SRC_DIR)/LightingEngine.h:

PROJECTEDSHADOW_H=$(SRC_DIR)/ProjectedShadow.h $(IDVFASTMATH_H) $(FILEACCESS_H) $(DEBUG_H)
$(SRC_DIR)/ProjectedShadow.h:

RANDOM_H=$(SRC_DIR)/Random.h
$(SRC_DIR)/Random.h:

SIMPLEBILLBOARD_H=$(SRC_DIR)/SimpleBillboard.h $(DEBUG_H) $(IDVVECTOR_H) $(UPVECTOR_H)
$(SRC_DIR)/SimpleBillboard.h:

SPEEDTREERT_H=$(SRC_DIR)/SpeedTreeRT.h
$(SRC_DIR)/SpeedTreeRT.h:

STRUCTSINFO_H=$(SRC_DIR)/StructsInfo.h $(IDVSPLINE_H) $(IDV_FILENAME_H)
$(SRC_DIR)/StructsInfo.h:

STRUCTSSUPPORT_H=$(SRC_DIR)/StructsSupport.h
$(SRC_DIR)/StructsSupport.h:

TREEENGINE_H=$(SRC_DIR)/TreeEngine.h $(IDVGLOBALS_H) $(IDVFILENAME_H) $(IDVVECTOR_H) $(IDVSPLINE_H) $(IDVRANDOM_H) $(STRUCTSINFO_H) $(STRUCTSSUPPORT_H) $(FILEACCESS_H)
$(SRC_DIR)/TreeEngine.h:

UPVECTOR_H=$(SRC_DIR)/UpVector.h
$(SRC_DIR)/UpVector.h:

WINDENGINE_H=$(SRC_DIR)/WindEngine.h $(SPEEDTREERT_H) $(STRUCTSINFO_H) $(DEBUG_H)
$(SRC_DIR)/WindEngine.h:

# LibFilename

IDVFILENAME_H=$(LIBFILENAME_SRC_DIR)/IdvFilename.h $(IDVGLOBALS_H)
$(LIBFILENAME_SRC_DIR)/IdvFilename.h:

# LibGlobals

IDVGLOBALS_H=$(LIBGLOBALS_SRC_DIR)/IdvGlobals.h
$(LIBGLOBALS_SRC_DIR)/IdvGlobals.h:

# LibRandom

EXTENDEDREAL_H=$(LIBRANDOM_SRC_DIR)/ExtendedReal.h
$(LIBRANDOM_SRC_DIR)/ExtendedReal.h:

IDVRANDOM_H=$(LIBRANDOM_SRC_DIR)/IdvRandom.h $(IDVGLOBALS_H)
$(LIBRANDOM_SRC_DIR)/IdvRandom.h:

ROBERTDAVIES_RANDOM_H=$(LIBRANDOM_SRC_DIR)/RobertDavies_Random.h $(EXTENDEDREAL_H)
$(LIBRANDOM_SRC_DIR)/RobertDavies_Random.h:

# LibSpline

IDVSPLINE_H=$(LIBSPLINE_SRC_DIR)/IdvSpline.h $(IDVVECTOR_H)
$(LIBSPLINE_SRC_DIR)/IdvSpline.h:

# LibVector

IDVFASTMATH_H=$(LIBVECTOR_SRC_DIR)/IdvFastMath.h
$(LIBVECTOR_SRC_DIR)/IdvFastMath.h:

IDVVECTOR_H=$(LIBVECTOR_SRC_DIR)/IdvVector.h $(IDVFASTMATH_H)
$(LIBVECTOR_SRC_DIR)/IdvVector.h:
