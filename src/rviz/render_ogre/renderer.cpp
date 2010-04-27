/*
 * Copyright (c) 2010, Willow Garage, Inc.
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

#include "renderer.h"
#include "render_window.h"
#include "scene.h"
#include "camera.h"

#include <rviz/render_interface/irender_loop_listener.h>

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneManager.h>

#include <ros/console.h>

using namespace rviz_uuid;

namespace rviz
{
namespace render
{
namespace ogre
{

Renderer::Renderer(const std::string& root_path, bool enable_ogre_log)
: running_(true)
, first_window_created_(false)
, root_path_(root_path)
, enable_ogre_log_(enable_ogre_log)
{
}

Renderer::~Renderer()
{
  stop();
}

void Renderer::start()
{
  running_ = true;
  render_thread_ = boost::thread(&Renderer::renderThread, this);
}

void Renderer::stop()
{
  running_ = false;
  render_thread_.join();

  delete Ogre::Root::getSingletonPtr();
}

void Renderer::init()
{
  Ogre::LogManager* log_manager = new Ogre::LogManager();
  log_manager->createLog( "Ogre.log", false, false, !enable_ogre_log_ );

  std::string plugin_cfg = "/etc/OGRE/plugins.cfg";
  bool has_plugin_cfg = false;
#ifdef HAS_INSTALLED_OGRE
  has_plugin_cfg = true;
#else
  plugin_cfg = "";
#endif

  Ogre::Root* root = new Ogre::Root( plugin_cfg );
  if ( !has_plugin_cfg )
  {
    root->loadPlugin( "RenderSystem_GL" );
    root->loadPlugin( "Plugin_OctreeSceneManager" );
    root->loadPlugin( "Plugin_ParticleFX" );
    root->loadPlugin( "Plugin_CgProgramManager" );
  }

  // Taken from gazebo
  Ogre::RenderSystem* render_system = NULL;
#if OGRE_VERSION_MAJOR >=1 && OGRE_VERSION_MINOR >= 7
  const Ogre::RenderSystemList& rsList = root->getAvailableRenderers();
  Ogre::RenderSystemList::const_iterator renderIt = rsList.begin();
  Ogre::RenderSystemList::const_iterator renderEnd = rsList.end();
#else
  Ogre::RenderSystemList* rsList = root->getAvailableRenderers();
  Ogre::RenderSystemList::iterator renderIt = rsList->begin();
  Ogre::RenderSystemList::iterator renderEnd = rsList->end();
#endif
  for ( ; renderIt != renderEnd; ++renderIt )
  {
    render_system = *renderIt;

    if ( render_system->getName() == "OpenGL Rendering Subsystem" )
    {
      break;
    }
  }

  if ( render_system == NULL )
  {
    throw std::runtime_error( "Could not find the opengl rendering subsystem!\n" );
  }

  render_system->setConfigOption("FSAA","2");
  render_system->setConfigOption("RTT Preferred Mode", "FBO");

  root->setRenderSystem( render_system );

  root->initialise( false );
}

void Renderer::oneTimeInit()
{
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation( root_path_ + "/ogre_media/textures", "FileSystem", ROS_PACKAGE_NAME );
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation( root_path_ + "/ogre_media/fonts", "FileSystem", ROS_PACKAGE_NAME );
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation( root_path_ + "/ogre_media/models", "FileSystem", ROS_PACKAGE_NAME );
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation( root_path_ + "/ogre_media/materials/programs", "FileSystem", ROS_PACKAGE_NAME );
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation( root_path_ + "/ogre_media/materials/scripts", "FileSystem", ROS_PACKAGE_NAME );
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

IRenderWindow* Renderer::createRenderWindow(const std::string& name, const std::string& parent_window, uint32_t width, uint32_t height)
{
  if (render_windows_.count(name) > 0)
  {
    throw std::runtime_error("Render window of name [" + name + "] already exists");
  }

  Ogre::Root* root = Ogre::Root::getSingletonPtr();
  Ogre::NameValuePairList params;
  params["parentWindowHandle"] = parent_window;

  Ogre::RenderWindow* win = root->createRenderWindow(name, width, height, false, &params);

  if (!first_window_created_)
  {
    oneTimeInit();
    first_window_created_ = true;
  }

  win->setActive(true);
  win->setVisible(true);
  win->setAutoUpdated(true);

  RenderWindowPtr ptr(new RenderWindow(name, win, this));
  render_windows_[name] = ptr;

  return ptr.get();
}

void Renderer::destroyRenderWindow(const std::string& name)
{
  M_RenderWindow::iterator it = render_windows_.find(name);
  if (it == render_windows_.end())
  {
    throw std::runtime_error("Tried to destroy render window [" + name + "] which does not exist");
  }

  const RenderWindowPtr& win = it->second;

  Ogre::Root* root = Ogre::Root::getSingletonPtr();
  Ogre::RenderWindow* ogre_win = win->getOgreRenderWindow();
  ogre_win->destroy();
  root->getRenderSystem()->destroyRenderWindow(ogre_win->getName());
}

IRenderWindow* Renderer::getRenderWindow(const std::string& name)
{
  M_RenderWindow::iterator it = render_windows_.find(name);
  if (it == render_windows_.end())
  {
    throw std::runtime_error("Render window [" + name + "] does not exist");
  }

  return it->second.get();
}

void Renderer::addRenderLoopListener(IRenderLoopListener* listener)
{
  render_loop_listeners_.push_back(listener);
}

void Renderer::removeRenderLoopListener(IRenderLoopListener* listener)
{
  V_RenderLoopListener::iterator it = std::find(render_loop_listeners_.begin(), render_loop_listeners_.end(), listener);
  if (it != render_loop_listeners_.end())
  {
    render_loop_listeners_.erase(it);
  }
}

IScene* Renderer::createScene(const UUID& id)
{
  if (scenes_.count(id) > 0)
  {
    ROS_WARN_STREAM("UUID " << id << " collided when creating a scene!");
    return 0;
  }

  Ogre::Root* root = Ogre::Root::getSingletonPtr();
  Ogre::SceneManager* scene_manager = root->createSceneManager(Ogre::ST_GENERIC);
  ScenePtr scene(new Scene(id, scene_manager));
  scenes_[id] = scene;

  return scene.get();
}

void Renderer::destroyScene(const UUID& id)
{
  M_Scene::iterator it = scenes_.find(id);
  if (it == scenes_.end())
  {
    std::stringstream ss;
    ss << "Scene " << id << " does not exist!";
    throw std::runtime_error(ss.str());
  }

  const ScenePtr& scene = it->second;
  Ogre::Root* root = Ogre::Root::getSingletonPtr();
  root->destroySceneManager(scene->getSceneManager());
}

IScene* Renderer::getScene(const UUID& id)
{
  M_Scene::iterator it = scenes_.find(id);
  if (it == scenes_.end())
  {
    std::stringstream ss;
    ss << "Scene " << id << " does not exist!";
    throw std::runtime_error(ss.str());
  }

  return it->second.get();
}

ICamera* Renderer::getCamera(const UUID& id)
{
  M_Scene::iterator it = scenes_.begin();
  M_Scene::iterator end = scenes_.end();
  for (; it != end; ++it)
  {
    const ScenePtr& scene = it->second;
    ICamera* cam = scene->getCamera(id);
    if (cam)
    {
      return cam;
    }
  }

  return 0;
}

void Renderer::renderThread()
{
  init();

  while (running_)
  {
    std::for_each(render_loop_listeners_.begin(), render_loop_listeners_.end(), boost::bind(&IRenderLoopListener::preRender, _1, this));

    Ogre::Root::getSingleton().renderOneFrame();

    std::for_each(render_loop_listeners_.begin(), render_loop_listeners_.end(), boost::bind(&IRenderLoopListener::postRender, _1, this));
  }

  delete Ogre::Root::getSingletonPtr();
}

} // namespace ogre
} // namespace render
} // namespace rviz