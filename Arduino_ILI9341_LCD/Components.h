/*
ILI9341 LCD Display controller for Arduino_Pedelec_Controller

Copyright (C) 2016
Andreas Butti, andreas.b242 at gmail dot com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#pragma once

#include "BaseComponent.h"

/**
 * List with the customized components
 */

//! Max. 10 Components on the screen (should be enough, there isn't more space)
#define COMPONENT_COUNT 10

class BaseComponent;

class Components {
  // Constructor / Destructor
public:
  //! Constructor
  Components();

  //! Destructor
  virtual ~Components();

  // public API
public:
   //! Draw all components
   void draw();

   //! Return the component at position index
   BaseComponent* get(uint8_t index);

   //! remove the element at index, but does not delete it
   void remove(uint8_t index);

 private:
   //! Update the Y position of all elements, and remove invisible elements from the list
   void updatePositionAndRemoveInvisible();
   
  // Member
private:
  //! List with the components
  BaseComponent* m_components[COMPONENT_COUNT];
};
