/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (Capabilites.h), the license terms are:

   You can redistribute it and/or  modify it under the terms of the GNU Lesser
   General Public License (LGPL) as published by the Free Software Foundation;
   either version 3 of the License, or (at your option) any later version.

   This programm is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this proramm; if not, see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

/** \file Capabilites.h
 *
 *  This file declares the cpabilites, a inverter might have.
 *  (Of course also the ones a inverter "have to have")
 *  The names defined are to be used for generation and lookup.
 *
 *  The strings are for "human information"
 *
 *
 *
 *  Created on: May 22, 2009
 *      Author: tobi
 */

#ifndef CAPABILITES_H_
#define CAPABILITES_H_


/** The Capabilites list has been updated.
 * This "pseudo" capabilites can be used to tell observers, that the
 * data provider might have detected new capabilites and therefore
 * check the presence of new interesting data.
 *
 * A common use is, if the InverterBase-client can do autodetection
 * of capabilities during runtime, or as a result of auto-detecting the
 * excact model of a Inverter Family.
 *
 * Every Observer should subscribe to this one.
 *
 *\note The associated concrete value is unused.
 *
 * \sa The pseudo cap "force unsubsribe" is related.
 *
 * THIS IS A MUST CAPABILITY -- EVERY INVERTER HAS THIS ONE.  */
#define CAPA_CAPAS_UPDATED  	 "CapabilityList Updated"
#define CAPA_CAPAS_UPDATED_TYPE  IValue::bool_type


/** Some Capabilites are now void and the observers have to unsubscribe.
 *
 * This "pseudo" capabilites can be used to tell observers, that the
 * data provider detected a situation, where the Capbailities List have
 * to be recreated.
 *
 * For this to happen safely, every Subscriber must remove itself from the
 * subscription list on every capability -- except on the remove and add pseudo
 * capability.
 *
 *\note The associated concrete value is unused.
 *
 * Common situation swhere this will be used:
 * 	- Configuration reload
 *  - Programm termination
 *
 * In case of a reload, a "CAPSA_CAPAS_UPDATED" will follow.
 *
 * \sa The pseudo cap "force unsubsribe" is related.
 *
 * THIS IS A MUST CAPABILITY -- EVERY INVERTER HAS THIS ONE.  */
#define CAPA_CAPAS_REMOVEALL 		"CapabilityList Updated"
#define CAPA_CAPAS_REMOVEALL_TYPE 	IValue::bool_type


/** Is data the provided by the inverter valid?
 * If the  inverter is "power down" (like the Sputnik at night), this
 * is the master switch telling that all data is now invalid.
 * (if the associated value is false)
 *
 * THIS IS A MUST CAPABILITY -- EVERY INVERTER HAS THIS ONE. */
#define CAPA_INVERTER_DATASTATE_NAME  "Data Validity"
#define CAPA_INVERTER_DATASTATE_TYPE  IValue::bool_type


/** Basic informations for the user -- these informations are usually not
 * updated. But, as a exception to this, a inverter class might do runtime
 * detection of these parameters, if the inverter supports them.
 *
 * This one is the "human readable" manufactor of the Inverter.
 */

#define CAPA_INVERTER_MANUFACTOR_NAME "Inverter Manufactor"
#define CAPA_INVERTER_MANUFACTOR_TYPE IValue::string_type


/** Basic informations for the user -- these informations are usually not
 * updated. But, as a exception to this, a inverter class might do runtime
 * detection of these parameters, if the inverter supports them.
 *
 * This one is the "human readable" model of the Inverter.
 */

#define CAPA_INVERTER_MODEL_NAME "Inverter Model"
#define CAPA_INVERTER_MODEL_TYPE IValue::string_type


/** Firmware version informations.
 *
 * If available, can contain a human-readable info about the
 * inverters firmware
 *
 * type: string
 *
 * Optional.
 *
*/

#define CAPA_INVERTER_FIRMWARE_NAME "Firmware Version"
#define CAPA_INVERTER_FIRMWARE_TYPE IValue::string_type


/** Total power feeding
 *
 * On inverters which feeds more than one phase, this is the
 * sum of all phases.
 *
 * Type: float
 *
 * Recommended for every inverter, but still optional
*/
#define CAPA_INVERTER_ACPOWER_TOTAL_NAME "Grid-Feeding-Power"
#define CAPA_INVERTER_ACPOWER_TOTAL_TYPE IValue::float_type

/** Power On Hours
 *
 * Counts the hours a inverter was powered.
 *
 * Type: float
 *
 * Optional.
*/
#define CAPA_INVERTER_PON_HOURS_NAME "PowerOnHours"
#define CAPA_INVERTER_PON_HOURS_TYPE IValue::float_type



/** Feeded Energy Y2D
 *
 * This year the inverter has produced this amount of energy. (kWh)
 *
 * Type: float
 *
 * Optional.
*/
#define CAPA_INVERTER_KWH_Y2D_NAME "Energy produced this year (kWh)"
#define CAPA_INVERTER_KWH_Y2D_TYPE IValue::float_type


/** Feeded Energy M2D
 *
 * This month the inverter has produced this amount of energy. (kWh)
 *
 * Type: float
 *
 * Optional.
*/
#define CAPA_INVERTER_KWH_M2D_NAME "Energy produced this month (kWh)"
#define CAPA_INVERTER_KWH_M2D_TYPE IValue::float_type

/** Feeded Energy Today
 *
 * Today the inverter has produced this amount of energy. (kWh)
 *
 * Type: float
 *
 * Optional.
*/
#define CAPA_INVERTER_KWH_2D_NAME "Energy produced today (kWh)"
#define CAPA_INVERTER_KWH_2D_TYPE IValue::float_type

/** Feeded Energy Total
 *
 * Today the inverter has produced this amount of energy. (kWh)
 *
 * Type: float
 *
 * Optional.
*/
#define CAPA_INVERTER_KWH_TOTAL_NAME "Energy produced (kWh)"
#define CAPA_INVERTER_KWH_TOTAL_TYPE IValue::float_type

/** Installed Power
 *
 * How much power has beein installed?
 *
 * At least on the Sputnik, this can be configured and read by a query.
 *
 * Others can set this by e.g configuration options.
 *
 * Type: Float
 *
 * Optional.
 *
 * */
#define CAPA_INVERTER_INSTALLEDPOWER_NAME "Installed solar power (kWp)"
#define CAPA_INVERTER_INSTALLEDPOWER_TYPE IValue::float_type

/** Current AC Power Frequency
 *
 *
 * Type: Float
 *
 * Optional.
 *
 * */
#define CAPA_INVERTER_NET_FREQUENCY_NAME "Net frequency (Hz)"
#define CAPA_INVERTER_NET_FREQUENCY_TYPE IValue::float_type

/** relative power
 *
 * The Sputnik offers what is called "relative Power".
 * TODO: Check whats relative about that.
 *
 * (However, this reading is somwhat vague, so it is probably
 * better calculated by PAC and PIN or PDC*IDC and PIN)
 *
 * Type: Float
 *
 * Optional.
 *
 * */
#define CAPA_INVERTER_RELPOWER_NAME "relative Power (%)"
#define CAPA_INVERTER_RELPOWER_TYPE IValue::float_type

/** DC Input Voltage
 *
 * Input voltage from the generator.
 *
 * Type: Float
 *
 * Optional.
 *
 * */
#define CAPA_INVERTER_INPUT_DC_VOLTAGE_NAME "DC VOLTAGE IN (V)"
#define CAPA_INVERTER_INPUT_DC_VOLTAGE_TYPE IValue::float_type

/** DC Input Current
 *
 * Input voltage from the generator.
 *
 * Type: Float
 *
 * Optional.
 *
 * */
#define CAPA_INVERTER_INPUT_DC_CURRENT_NAME "DC CURRENT IN (A)"
#define CAPA_INVERTER_INPUT_DC_CURRENT_TYPE IValue::float_type


/** AC Grid Voltage
 *
 * For single-phase inverters: Grid voltage of the (single) phase.
 *
 * Type: Float
 *
 * Optional.
 *
 * */
#define CAPA_INVERTER_GRID_AC_VOLTAGE_NAME "AC GRID VOLTAGE (V)"
#define CAPA_INVERTER_GRID_AC_VOLTAGE_TYPE IValue::float_type

/** AC Grid Current
 *
 * For single-phase inverters: Grid voltage of the (single) phase.
 *
 * Type: Float
 *
 * Optional.
 *
 * */
#define CAPA_INVERTER_GRID_AC_CURRENT_NAME "AC GRID CURRENT OUT (V)"
#define CAPA_INVERTER_GRID_AC_CURRENT_TYPE IValue::float_type

/** Inverter internal temperature
 *
 * if supported, this shows the temp of the inverter.
 * (and one some models, if the fan is on.)
 *
 * (Note: If you prefer °F, you can programm a filter to
 * transform it...)
 *
 * Type: Float
 *
 * Optional.
 *
 * */
#define CAPA_INVERTER_TEMPERATURE_NAME "INVERTER TEMPERATURE (°C)"
#define CAPA_INVERTER_TEMPERATURE_TYPE IValue::float_type


#endif /* CAPABILITES_H_ */
