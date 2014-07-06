/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

 Copyright (C) 2009-2014 Tobias Frost

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
 */

/*
 * \file CDBWHSpecialTokens.h
 *
 *  Created on: 06.07.2014
 *      Author: tobi
 *
 * This classes handles the "special database tokens", starting with "%"
 *
 * The classes itself using the IValue interface.
 */

#ifndef CDBWHSPECIALTOKENS_H_
#define CDBWHSPECIALTOKENS_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef  HAVE_FILTER_DBWRITER

#include <ctime>
#include "patterns/CValue.h"

bool operator==(struct tm t1, struct tm t2);
bool operator!=(struct tm t1, struct tm t2);

// Template specialisation required for tm
template<>
class CValue<struct tm> : public IValue {

public:
    CValue()
    {
        IValue::type_ = MagicNumbers::magic_number_for<struct tm>();
    }

    CValue(const struct tm &set) {
         IValue::type_ = MagicNumbers::magic_number_for<struct tm>();
         value = set;
         timestamp = boost::posix_time::second_clock::local_time();
     }

    /// Serves as a virtual copy constructor.
    virtual CValue<struct tm>* clone() {
        return new CValue<struct tm>(*this);
    }

    void Set(struct tm value, boost::posix_time::ptime timestamp = boost::posix_time::second_clock::local_time()) {
         this->timestamp = timestamp;
         this->value = value;
     }

    struct tm Get(void) const {
         return value;
     }

     virtual void operator=(const struct tm& val) {
         timestamp = boost::posix_time::second_clock::local_time();
         value = val;
     }

     virtual void operator=(const CValue<struct tm> &val) {
         timestamp = val.GetTimestamp();
         value = val.Get();
     }

     virtual boost::posix_time::ptime GetTimestamp(void) const {
         return timestamp;
     }

     virtual operator std::string() {
         char tmp[128];
         std::string ret;
         std::strftime(tmp,sizeof(tmp),"%c %Z",&value);
         ret = tmp;
         return ret;
     }

     virtual bool operator==(IValue &v) {
         if (GetInternalType() == v.GetInternalType()) {
             CValue<struct tm> &realv = (CValue<struct tm>&)v;
             return (realv.Get() == Get());
         }
         return false;
     }

     virtual bool operator!=(IValue &v) {
          if (GetInternalType() == v.GetInternalType()) {
              CValue<struct tm> &realv = (CValue<struct tm>&)v;
              return (realv.Get() != Get());
          }
          return false;
      }

     /** Static interface function to determine at runtime the type of the CValue
      * object.
      * Usage example:
      * CValue<int> cv_int;
      * IValue *iv1 = &cv_int;
      * cout << CValue<int>::IsType(iv1);*/
     static bool IsType(IValue *totest) {
         if (MagicNumbers::magic_number_for<struct tm>() == totest->GetInternalType()) {
             return true;
         }
         return false;
     }


private:
    struct tm value;
    boost::posix_time::ptime timestamp;

};


template<class T>
class IDBHSpecialToken : public CValue<T>
{
public:
    /// Update the underlaying CValue<T>
    /// \return true if value indeed chagned, false if unchanged.
    virtual bool Update(const struct tm &tm) = 0;

};

class CDBHST_Timestamp : public IDBHSpecialToken<struct tm> {
public:
    virtual bool Update(const struct tm &tm);
};

class CDBHST_Year : public IDBHSpecialToken<long> {
public:
    virtual bool Update(const struct tm &tm);
};

class CDBHST_Month : public IDBHSpecialToken<long> {
public:
    virtual bool Update(const struct tm &tm);
};

class CDBHST_Day : public IDBHSpecialToken<long> {
public:
    virtual bool Update(const struct tm &tm);
};

class CDBHST_Hour : public IDBHSpecialToken<long> {
public:
    virtual bool Update(const struct tm &tm);
};

class CDBHST_Minute : public IDBHSpecialToken<long> {
public:
    virtual bool Update(const struct tm &tm);
};


class CDBHSpecialTokenFactory {
public:
    virtual IValue *Factory(const std::string &id) {

        if (id == "%TIMESTAMP") return new CDBHST_Timestamp;
        if (id == "%YEAR") return new CDBHST_Year;
        if (id == "%MONTH") return new CDBHST_Month;
       // if (id == "%WEEK") return new CDBHST_Week;
        if (id == "%DAY") return new CDBHST_Day;
        if (id == "%HOUR") return new CDBHST_Hour;
        if (id == "%MINUTE") return new CDBHST_Minute;
        return NULL;
    }
};



#endif

#endif /* CDBWHSPECIALTOKENS_H_ */
