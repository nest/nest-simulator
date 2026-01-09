/*
 *  connection_label.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CONNECTION_LABEL_H
#define CONNECTION_LABEL_H

#include "dictdatum.h"
#include "dictutils.h"
#include "exceptions.h"
#include "nest_names.h"

namespace nest
{
class ConnectorModel;

/**
 * Connections are unlabeled by default. Unlabeled connections cannot be
 * specified
 * as a search criterion in the `GetConnections` function.
 * @see ConnectionLabel
 */
const static long UNLABELED_CONNECTION = -1;

} // namespace nest

#endif /* CONNECTION_LABEL_H */
