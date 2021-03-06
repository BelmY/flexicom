/* 
 * This file is part of the FlexiCom distribution (https://github.com/polhenarejos/flexicom).
 * Copyright (c) 2012-2020 Pol Henarejos, at Centre Tecnologic de Telecomunicacions de Catalunya (CTTC).
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/* $Id$ */
/* $Format:%ci$ ($Format:%h$) by $Format:%an%$ */

#ifndef _INC_COMPAT_H_
#define _INC_COMPAT_H_

#ifdef _WIN
#define __attribute__(x)
#define PACK( __Declaration__ , __Name__ ) __pragma( pack(push, 1) ) ; typedef __Declaration__ __Name__ ; __pragma( pack(pop) )
#else
#define PACK( __Declaration__ , __Name__ ) typedef __Declaration__ __attribute__((__packed__)) __Name__
#endif

#endif //_INC_COMPAT_H_
