/*
 * Copyright (c) 2008, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "convert.h"

namespace rviz
{
namespace render
{
namespace ogre
{

static Ogre::Matrix3 fromEulersYXZ(float y, float x, float z)
{
  Ogre::Matrix3 m;
  m.FromEulerAnglesYXZ(Ogre::Degree(y), Ogre::Degree(x), Ogre::Degree(z));
  return m;
}

static Ogre::Quaternion matToQuat(const Ogre::Matrix3& mat)
{
  Ogre::Quaternion q;
  q.FromRotationMatrix(mat);
  return q;
}

Ogre::Matrix3 g_ogre_to_robot_matrix(fromEulersYXZ(-90, 0, -90));
Ogre::Matrix3 g_robot_to_ogre_matrix(g_ogre_to_robot_matrix.Inverse());

Ogre::Quaternion g_ogre_to_robot_quat(matToQuat(g_ogre_to_robot_matrix));
Ogre::Quaternion g_robot_to_ogre_quat(matToQuat(g_robot_to_ogre_matrix));

} // namespace ogre
} // nmaespace render
} // namespace rviz