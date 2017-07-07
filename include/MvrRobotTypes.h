#ifndef MVRROBOTTYPES_H
#define MVRROBOTTYPES_H


#include "mvriaUtil.h"
#include "MvrRobotParams.h"

/** @cond INCLUDE_INTERNAL_ROBOT_PARAM_CLASSES */

class MvrRobotGeneric : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotGeneric(const char *dir="");
  MVREXPORT virtual ~MvrRobotGeneric() {}
};

class MvrRobotAmigo : public MvrRobotParams
{
public:

  MVREXPORT MvrRobotAmigo(const char *dir="");
  MVREXPORT virtual ~MvrRobotAmigo() {}
};

class MvrRobotAmigoSh : public MvrRobotParams
{
public:

  MVREXPORT MvrRobotAmigoSh(const char *dir="");
  MVREXPORT virtual ~MvrRobotAmigoSh() {}
};


class MvrRobotAmigoShTim5xxWibox : public MvrRobotAmigoSh
{
public:
  MVREXPORT MvrRobotAmigoShTim5xxWibox(const char *dir="");
  MVREXPORT virtual ~MvrRobotAmigoShTim5xxWibox() {}
};

class MvrRobotP2AT : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2AT(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2AT() {}
};

class MvrRobotP2AT8 : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2AT8(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2AT8() {}
};

class MvrRobotP2AT8Plus : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2AT8Plus(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2AT8Plus() {}
};

class MvrRobotP2IT : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2IT(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2IT() {}
};

class MvrRobotP2DX : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2DX(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2DX() {}
};

class MvrRobotP2DXe : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2DXe(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2DXe() {}
};

class MvrRobotP2DF : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2DF(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2DF() {}
};

class MvrRobotP2D8 : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2D8(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2D8() {}
};

class MvrRobotP2D8Plus : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2D8Plus(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2D8Plus() {}
};

class MvrRobotP2CE : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2CE(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2CE() {}
};

class MvrRobotP2PP : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2PP(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2PP() {}
};

class MvrRobotP2PB : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP2PB(const char *dir="");
  MVREXPORT virtual ~MvrRobotP2PB() {}
};


class MvrRobotP3AT : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP3AT(const char *dir="");
  MVREXPORT virtual ~MvrRobotP3AT() {}
};


class MvrRobotP3DX : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotP3DX(const char *dir="");
  MVREXPORT virtual ~MvrRobotP3DX() {}
};

class MvrRobotPerfPB : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotPerfPB(const char *dir="");
  MVREXPORT virtual ~MvrRobotPerfPB() {}
};

class MvrRobotPerfPBPlus : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotPerfPBPlus(const char *dir="");
  MVREXPORT virtual ~MvrRobotPerfPBPlus() {}
};

class MvrRobotPion1M : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotPion1M(const char *dir="");
  MVREXPORT virtual ~MvrRobotPion1M() {}
};

class MvrRobotPion1X : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotPion1X(const char *dir="");
  MVREXPORT virtual ~MvrRobotPion1X() {}
};

class MvrRobotPsos1M : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotPsos1M(const char *dir="");
  MVREXPORT virtual ~MvrRobotPsos1M() {}
};

class MvrRobotPsos43M : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotPsos43M(const char *dir="");
  MVREXPORT virtual ~MvrRobotPsos43M() {}
};

class MvrRobotPsos1X : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotPsos1X(const char *dir="");
  MVREXPORT virtual ~MvrRobotPsos1X() {}
};

class MvrRobotPionAT : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotPionAT(const char *dir="");
  MVREXPORT virtual ~MvrRobotPionAT() {}
};

class MvrRobotMapper : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotMapper(const char *dir="");
  MVREXPORT virtual ~MvrRobotMapper() {}
};

class MvrRobotPowerBot : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotPowerBot(const char *dir="");
  MVREXPORT virtual ~MvrRobotPowerBot() {}
};

class MvrRobotP3DXSH : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotP3DXSH(const char *dir="");
  MVREXPORT virtual ~MvrRobotP3DXSH() {}
};

class MvrRobotP3ATSH : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotP3ATSH(const char *dir="");
  MVREXPORT virtual ~MvrRobotP3ATSH() {}
};

class MvrRobotP3ATIWSH : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotP3ATIWSH(const char *dir="");
  MVREXPORT virtual ~MvrRobotP3ATIWSH() {}
};

class MvrRobotPatrolBotSH : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotPatrolBotSH(const char *dir="");
  MVREXPORT virtual ~MvrRobotPatrolBotSH() {}
};

class MvrRobotPeopleBotSH : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotPeopleBotSH(const char *dir="");
  MVREXPORT virtual ~MvrRobotPeopleBotSH() {}
};

class MvrRobotPowerBotSH : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotPowerBotSH(const char *dir="");
  MVREXPORT virtual ~MvrRobotPowerBotSH() {}
};

class MvrRobotWheelchairSH : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotWheelchairSH(const char *dir="");
  MVREXPORT virtual ~MvrRobotWheelchairSH() {}
};

class MvrRobotPowerBotSHuARCS : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotPowerBotSHuARCS(const char *dir="");
  MVREXPORT virtual ~MvrRobotPowerBotSHuARCS() {}
};

class MvrRobotSeekur : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotSeekur(const char *dir="");
  MVREXPORT virtual ~MvrRobotSeekur() {}
};

/// @since Mvr 2.7.2
class MvrRobotMT400 : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotMT400(const char *dir="");
  MVREXPORT virtual ~MvrRobotMT400() {}
};

/// @since Mvr 2.7.2
class MvrRobotResearchPB : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotResearchPB(const char *dir="");
  MVREXPORT virtual ~MvrRobotResearchPB() {}
};

/// @since Mvr 2.7.2
class MvrRobotSeekurJr : public MvrRobotParams
{
 public:
  MVREXPORT MvrRobotSeekurJr(const char *dir="");
  MVREXPORT virtual ~MvrRobotSeekurJr() {}
};

/// @since Mvr 2.7.4
class MvrRobotP3DXSH_lms1xx : public MvrRobotP3DXSH
{
public: 
  MVREXPORT MvrRobotP3DXSH_lms1xx(const char *dir="");
};

/// @since Mvr 2.7.4
class MvrRobotP3ATSH_lms1xx : public MvrRobotP3ATSH
{
public: 
  MVREXPORT MvrRobotP3ATSH_lms1xx(const char *dir="");
};

/// @since Mvr 2.7.4
class MvrRobotPeopleBotSH_lms1xx : public MvrRobotPeopleBotSH
{
public: 
  MVREXPORT MvrRobotPeopleBotSH_lms1xx(const char *dir="");
};

/// @since Mvr 2.7.4
class MvrRobotP3DXSH_lms500 : public MvrRobotP3DXSH
{
public: 
  MVREXPORT MvrRobotP3DXSH_lms500(const char *dir="");
};

/// @since Mvr 2.7.4
class MvrRobotP3ATSH_lms500 : public MvrRobotP3ATSH
{
public: 
  MVREXPORT MvrRobotP3ATSH_lms500(const char *dir="");
};

/// @since Mvr 2.7.4
class MvrRobotPeopleBotSH_lms500 : public MvrRobotPeopleBotSH
{
public: 
  MVREXPORT MvrRobotPeopleBotSH_lms500(const char *dir="");
};

/// @since Mvr 2.7.4
class MvrRobotPowerBotSH_lms500 : public MvrRobotPowerBotSH
{
public: 
  MVREXPORT MvrRobotPowerBotSH_lms500(const char *dir="");
};

/// @since Mvr 2.7.4
class MvrRobotResearchPB_lms500 : public MvrRobotResearchPB
{
public: 
  MVREXPORT MvrRobotResearchPB_lms500(const char *dir="");
};

/// @since Mvr 2.8
class MvrRobotPioneerLX : public MvrRobotParams
{
public:
  MVREXPORT MvrRobotPioneerLX(const char *dir="");
  MVREXPORT virtual ~MvrRobotPioneerLX() {}
};

/** @endcond INCLUDE_INTERNAL_ROBOT_PARAM_CLASSES */

#endif // ARROBOTTYPES_H
