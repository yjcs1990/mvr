/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/
#ifndef ARROBOTTYPES_H
#define ARROBOTTYPES_H


#include "ariaUtil.h"
#include "MvrRobotParams.h"

/** @cond INCLUDE_INTERNAL_ROBOT_PARAM_CLASSES */

class MvrRobotGeneric : public MvrRobotParams
{
public:
  AREXPORT MvrRobotGeneric(const char *dir="");
  AREXPORT virtual ~ArRobotGeneric() {}
};

class MvrRobotAmigo : public MvrRobotParams
{
public:

  AREXPORT MvrRobotAmigo(const char *dir="");
  AREXPORT virtual ~ArRobotAmigo() {}
};

class MvrRobotAmigoSh : public MvrRobotParams
{
public:

  AREXPORT MvrRobotAmigoSh(const char *dir="");
  AREXPORT virtual ~ArRobotAmigoSh() {}
};


class MvrRobotAmigoShTim5xxWibox : public MvrRobotAmigoSh
{
public:
  AREXPORT MvrRobotAmigoShTim5xxWibox(const char *dir="");
  AREXPORT virtual ~ArRobotAmigoShTim5xxWibox() {}
};

class MvrRobotP2AT : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2AT(const char *dir="");
  AREXPORT virtual ~ArRobotP2AT() {}
};

class MvrRobotP2AT8 : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2AT8(const char *dir="");
  AREXPORT virtual ~ArRobotP2AT8() {}
};

class MvrRobotP2AT8Plus : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2AT8Plus(const char *dir="");
  AREXPORT virtual ~ArRobotP2AT8Plus() {}
};

class MvrRobotP2IT : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2IT(const char *dir="");
  AREXPORT virtual ~ArRobotP2IT() {}
};

class MvrRobotP2DX : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2DX(const char *dir="");
  AREXPORT virtual ~ArRobotP2DX() {}
};

class MvrRobotP2DXe : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2DXe(const char *dir="");
  AREXPORT virtual ~ArRobotP2DXe() {}
};

class MvrRobotP2DF : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2DF(const char *dir="");
  AREXPORT virtual ~ArRobotP2DF() {}
};

class MvrRobotP2D8 : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2D8(const char *dir="");
  AREXPORT virtual ~ArRobotP2D8() {}
};

class MvrRobotP2D8Plus : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2D8Plus(const char *dir="");
  AREXPORT virtual ~ArRobotP2D8Plus() {}
};

class MvrRobotP2CE : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2CE(const char *dir="");
  AREXPORT virtual ~ArRobotP2CE() {}
};

class MvrRobotP2PP : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2PP(const char *dir="");
  AREXPORT virtual ~ArRobotP2PP() {}
};

class MvrRobotP2PB : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP2PB(const char *dir="");
  AREXPORT virtual ~ArRobotP2PB() {}
};


class MvrRobotP3AT : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP3AT(const char *dir="");
  AREXPORT virtual ~ArRobotP3AT() {}
};


class MvrRobotP3DX : public MvrRobotParams
{
public:
  AREXPORT MvrRobotP3DX(const char *dir="");
  AREXPORT virtual ~ArRobotP3DX() {}
};

class MvrRobotPerfPB : public MvrRobotParams
{
public:
  AREXPORT MvrRobotPerfPB(const char *dir="");
  AREXPORT virtual ~ArRobotPerfPB() {}
};

class MvrRobotPerfPBPlus : public MvrRobotParams
{
public:
  AREXPORT MvrRobotPerfPBPlus(const char *dir="");
  AREXPORT virtual ~ArRobotPerfPBPlus() {}
};

class MvrRobotPion1M : public MvrRobotParams
{
public:
  AREXPORT MvrRobotPion1M(const char *dir="");
  AREXPORT virtual ~ArRobotPion1M() {}
};

class MvrRobotPion1X : public MvrRobotParams
{
public:
  AREXPORT MvrRobotPion1X(const char *dir="");
  AREXPORT virtual ~ArRobotPion1X() {}
};

class MvrRobotPsos1M : public MvrRobotParams
{
public:
  AREXPORT MvrRobotPsos1M(const char *dir="");
  AREXPORT virtual ~ArRobotPsos1M() {}
};

class MvrRobotPsos43M : public MvrRobotParams
{
public:
  AREXPORT MvrRobotPsos43M(const char *dir="");
  AREXPORT virtual ~ArRobotPsos43M() {}
};

class MvrRobotPsos1X : public MvrRobotParams
{
public:
  AREXPORT MvrRobotPsos1X(const char *dir="");
  AREXPORT virtual ~ArRobotPsos1X() {}
};

class MvrRobotPionAT : public MvrRobotParams
{
public:
  AREXPORT MvrRobotPionAT(const char *dir="");
  AREXPORT virtual ~ArRobotPionAT() {}
};

class MvrRobotMapper : public MvrRobotParams
{
public:
  AREXPORT MvrRobotMapper(const char *dir="");
  AREXPORT virtual ~ArRobotMapper() {}
};

class MvrRobotPowerBot : public MvrRobotParams
{
public:
  AREXPORT MvrRobotPowerBot(const char *dir="");
  AREXPORT virtual ~ArRobotPowerBot() {}
};

class MvrRobotP3DXSH : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotP3DXSH(const char *dir="");
  AREXPORT virtual ~ArRobotP3DXSH() {}
};

class MvrRobotP3ATSH : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotP3ATSH(const char *dir="");
  AREXPORT virtual ~ArRobotP3ATSH() {}
};

class MvrRobotP3ATIWSH : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotP3ATIWSH(const char *dir="");
  AREXPORT virtual ~ArRobotP3ATIWSH() {}
};

class MvrRobotPatrolBotSH : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotPatrolBotSH(const char *dir="");
  AREXPORT virtual ~ArRobotPatrolBotSH() {}
};

class MvrRobotPeopleBotSH : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotPeopleBotSH(const char *dir="");
  AREXPORT virtual ~ArRobotPeopleBotSH() {}
};

class MvrRobotPowerBotSH : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotPowerBotSH(const char *dir="");
  AREXPORT virtual ~ArRobotPowerBotSH() {}
};

class MvrRobotWheelchairSH : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotWheelchairSH(const char *dir="");
  AREXPORT virtual ~ArRobotWheelchairSH() {}
};

class MvrRobotPowerBotSHuARCS : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotPowerBotSHuARCS(const char *dir="");
  AREXPORT virtual ~ArRobotPowerBotSHuARCS() {}
};

class MvrRobotSeekur : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotSeekur(const char *dir="");
  AREXPORT virtual ~ArRobotSeekur() {}
};

/// @since Mvria 2.7.2
class MvrRobotMT400 : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotMT400(const char *dir="");
  AREXPORT virtual ~ArRobotMT400() {}
};

/// @since Mvria 2.7.2
class MvrRobotResearchPB : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotResearchPB(const char *dir="");
  AREXPORT virtual ~ArRobotResearchPB() {}
};

/// @since Mvria 2.7.2
class MvrRobotSeekurJr : public MvrRobotParams
{
 public:
  AREXPORT MvrRobotSeekurJr(const char *dir="");
  AREXPORT virtual ~ArRobotSeekurJr() {}
};

/// @since Mvria 2.7.4
class MvrRobotP3DXSH_lms1xx : public MvrRobotP3DXSH
{
public: 
  AREXPORT MvrRobotP3DXSH_lms1xx(const char *dir="");
};

/// @since Mvria 2.7.4
class MvrRobotP3ATSH_lms1xx : public MvrRobotP3ATSH
{
public: 
  AREXPORT MvrRobotP3ATSH_lms1xx(const char *dir="");
};

/// @since Mvria 2.7.4
class MvrRobotPeopleBotSH_lms1xx : public MvrRobotPeopleBotSH
{
public: 
  AREXPORT MvrRobotPeopleBotSH_lms1xx(const char *dir="");
};

/// @since Mvria 2.7.4
class MvrRobotP3DXSH_lms500 : public MvrRobotP3DXSH
{
public: 
  AREXPORT MvrRobotP3DXSH_lms500(const char *dir="");
};

/// @since Mvria 2.7.4
class MvrRobotP3ATSH_lms500 : public MvrRobotP3ATSH
{
public: 
  AREXPORT MvrRobotP3ATSH_lms500(const char *dir="");
};

/// @since Mvria 2.7.4
class MvrRobotPeopleBotSH_lms500 : public MvrRobotPeopleBotSH
{
public: 
  AREXPORT MvrRobotPeopleBotSH_lms500(const char *dir="");
};

/// @since Mvria 2.7.4
class MvrRobotPowerBotSH_lms500 : public MvrRobotPowerBotSH
{
public: 
  AREXPORT MvrRobotPowerBotSH_lms500(const char *dir="");
};

/// @since Mvria 2.7.4
class MvrRobotResearchPB_lms500 : public MvrRobotResearchPB
{
public: 
  AREXPORT MvrRobotResearchPB_lms500(const char *dir="");
};

/// @since Mvria 2.8
class MvrRobotPioneerLX : public MvrRobotParams
{
public:
  AREXPORT MvrRobotPioneerLX(const char *dir="");
  AREXPORT virtual ~ArRobotPioneerLX() {}
};

/** @endcond INCLUDE_INTERNAL_ROBOT_PARAM_CLASSES */

#endif // ARROBOTTYPES_H
