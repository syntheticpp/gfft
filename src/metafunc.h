/***************************************************************************
 *   Copyright (C) 2007-2014 by Vladimir Mirnyy                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ***************************************************************************/

#ifndef __metafunc_h
#define __metafunc_h

/** \file
    \brief Compile-time computing of mathematical functions
*/

#include <cmath>

#include "srational.h"
#include "sdecimal.h"

#include "pseudometafunc.h"


template<class First, class Second>
struct EmptyOperation {
  typedef Second Result;
};


// Works with SRational of decimal bases (10^n) only
// TODO: change that
template<class Rational, int_t NDigits, base_t DecBase=DefaultDecimalBase>
struct RationalToDecimal;

template<class Numer, class Denom, int_t NDigits, base_t DecBase>
struct RationalToDecimal<SRational<Numer,Denom>,NDigits,DecBase> {
  typedef typename MF::IPowBig<SInt<DecBase>,NDigits>::Result D;
  typedef typename Mult<Numer,D>::Result NewNumer;
  typedef typename Div<NewNumer,Denom>::DivResult AllDecimals;
  typedef SDecimal<AllDecimals,NDigits,DecBase> Result;
};

template<bool S1, class N1, class Denom, int_t NDigits, base_t Base>
struct RationalToDecimal<SRational<SBigInt<S1,N1,Base>,Denom>,NDigits,Base> {
  typedef typename Loki::TL::ShiftRight<N1,NDigits,SInt<0> >::Result NList;
  typedef SBigInt<S1,NList,Base> NewNumer;
//typedef typename NL::Print<NewNumer>::Result TT2;
  typedef typename Div<NewNumer,Denom>::DivResult AllDecimals;
  typedef SDecimal<AllDecimals,NDigits,Base> Result;
};

template<int_t N, int_t NDigits, base_t DecBase>
struct RationalToDecimal<SInt<N>,NDigits,DecBase> {
  typedef SDecimal<SInt<N>,0,DecBase> Result;
};

template<class BI, int_t ND, base_t Base, int_t NDigits, base_t DecBase>
struct RationalToDecimal<SDecimal<BI,ND,Base>,NDigits,DecBase> {
  typedef BI AllDecimals;
  typedef SDecimal<BI,ND,Base> Result;
};

/////////////////////////////////////////////////////

template<class N, class D, int Accuracy, base_t Base>
struct Reduce<SRational<N,D>,Accuracy,Base> {
  typedef typename MF::IPowBig<SInt<Base>,Accuracy>::Result Denom;
  typedef typename RationalToDecimal<SRational<N,D>,Accuracy,Base>::AllDecimals Decimals;
  typedef typename Simplify<SRational<Decimals,Denom> >::Result Result;
};


/// Metafunctions template classes
/*!
Template classes under this namespace are dedicated to the calculation of
different mathematical functions at compile time. Return values can be static constants, 
if they can be represented as integers, or static functions returning floating point values.
*/
namespace MF {


template<class X, class FuncStep,
template<class,class> class Accum,
int Accuracy,
int_t Count> 
struct FuncSeries
{
  typedef FuncSeries<X,FuncStep,Accum,Accuracy,Count-1> NextIter;
  typedef typename FuncStep::template Value<Count-1,X,typename NextIter::ResultAux,Accuracy> FStep;
  typedef typename FStep::Result Step;
  typedef typename FStep::ResultAux ResultAux;

  typedef typename Accum<Step,typename NextIter::Result>::Result Result;
};

template<class X, class FuncStep,
template<class,class> class Accum,
int Accuracy> 
struct FuncSeries<X,FuncStep,Accum,Accuracy,1>
{
  typedef typename FuncStep::template Value<0,X,Loki::NullType,Accuracy> FStep;
  typedef typename FStep::Result Result;
  typedef typename FStep::ResultAux ResultAux;
};

template<class X, class FuncStep,
template<class,class> class Accum, 
int Accuracy> 
struct FuncSeries<X,FuncStep,Accum,Accuracy,0> {};  // Error

/////////////////////////////////////////////////////

template<class X, class FuncStep,
template<class,class> class Accum,
int Accuracy, int I, 
class Value, class Dec1, class Dec2, class Aux,
bool C = (NL::Compare<Dec1,Dec2>::value == 0)>
struct FuncAccuracyLoop;

template<class X, class FuncStep,
template<class,class> class Accum,
int Accuracy, int I, class Value, class Dec1, class Dec2, class Aux>
struct FuncAccuracyLoop<X,FuncStep,Accum,Accuracy,I,Value,Dec1,Dec2,Aux,true>
{
  typedef Dec2 NextDecimal;
  typedef Value Result;
};

template<class X, class FuncStep,
template<class,class> class Accum,
int Accuracy, int I, class Value, class Dec1, class Dec2, class Aux>
struct FuncAccuracyLoop<X,FuncStep,Accum,Accuracy,I,Value,Dec1,Dec2,Aux,false>
{
  typedef typename FuncStep::template Value<I,X,Aux,Accuracy> FStep;
  typedef typename FStep::Result NextStep;
  typedef typename FStep::ResultAux NextAux;
  typedef typename Accum<Value,NextStep>::Result NextValue;
  
  typedef typename RationalToDecimal<NextValue,Accuracy,DefaultDecimalBase>::AllDecimals NextDecimal;
  typedef typename FuncAccuracyLoop<X,FuncStep,Accum,Accuracy,I+1,NextValue,Dec2,NextDecimal,NextAux>::Result Result;
};

/////////////////////////////////////////////////////

template<class X, class FuncStep,
template<class,class> class Accum,
int Len, int I, class Value1, class Value2, class Aux,
bool C = (NL::Length<typename Value2::Numer>::value > Len 
       || NL::Length<typename Value2::Denom>::value > Len)>
struct FuncLengthLoop;

template<class X, class FuncStep,
template<class,class> class Accum,
int Len, int I, class Value1, class Value2, class Aux>
struct FuncLengthLoop<X,FuncStep,Accum,Len,I,Value1,Value2,Aux,true>
{
  typedef Value1 Result;
};

template<class X, class FuncStep,
template<class,class> class Accum,
int Len, int I, class Value1, class Value2, class Aux>
struct FuncLengthLoop<X,FuncStep,Accum,Len,I,Value1,Value2,Aux,false>
{
  typedef typename FuncStep::template Value<I,X,Aux> FStep;
  typedef typename FStep::Result NextStep;
  typedef typename FStep::ResultAux NextAux;
  typedef typename Accum<Value2,NextStep>::Result NextValue;
  typedef typename FuncLengthLoop<X,FuncStep,Accum,Len,I+1,Value2,NextValue,NextAux>::Result Result;
};

/////////////////////////////////////////////////////
template<class Value, int Accuracy, base_t Base>
struct GenericAccuracyBasedFuncAdapter;

template<class N, class D, int Accuracy, base_t Base>
struct GenericAccuracyBasedFuncAdapter<SRational<N,D>,Accuracy,Base>
{
  typedef typename RationalToDecimal<SRational<N,D>,Accuracy,DefaultBase>::AllDecimals Result;
};

template<class BI, int_t ND, int Accuracy, base_t Base>
struct GenericAccuracyBasedFuncAdapter<SDecimal<BI,ND,Base>,Accuracy,Base>
{
  typedef typename SDecimal<BI,ND,Base>::Num Result;
};


template<class X, class FuncStep,  // One step of the series to compute the function
template<class,class> class Accumulator,      // How the steps are accumulated, normally Add or Mult
int Accuracy,                 // in powers of DefaultBase
int NStartingSteps>
struct GenericAccuracyBasedFunc 
{
  typedef FuncSeries<X,FuncStep,Accumulator,Accuracy,NStartingSteps> Sum;
  typedef typename Sum::Result StartValue;
  typedef typename Sum::ResultAux Aux;
  typedef typename GenericAccuracyBasedFuncAdapter<StartValue,Accuracy,DefaultBase>::Result StartDecimal;

  typedef typename FuncStep::template Value<NStartingSteps,X,Aux,Accuracy> FStep;
  typedef typename FStep::Result NextStep;
  typedef typename FStep::ResultAux NextAux;
  typedef typename Accumulator<StartValue,NextStep>::Result NextValue;     // sequence of parameters is important here !!!
  typedef typename GenericAccuracyBasedFuncAdapter<NextValue,Accuracy,DefaultBase>::Result NextDecimal;
  
  typedef typename FuncAccuracyLoop<X,FuncStep,Accumulator,Accuracy,NStartingSteps+1,
                                    NextValue,StartDecimal,NextDecimal,NextAux>::Result Result;
};

/////////////////////////////////////////////////////

template<class X, class FuncStep,  // One step of the series to compute the function
template<class,class> class Accumulator,      // How the steps are accumulated, normally Add or Mult
int Length,    // in digits of DefaultBase
int NStartingSteps>
struct GenericLengthBasedFunc
{
  typedef FuncSeries<X,FuncStep,Accumulator,Length,NStartingSteps> Sum;
  typedef typename Sum::Result StartValue;
  typedef typename Sum::ResultAux Aux;

  typedef typename FuncStep::template Value<NStartingSteps,X,Aux,Length> FStep;
  typedef typename FStep::Result NextStep;
  typedef typename FStep::ResultAux NextAux;
  typedef typename Accumulator<NextStep,StartValue>::Result NextValue;

  typedef typename FuncLengthLoop<X,FuncStep,Accumulator,Length,NStartingSteps+1,StartValue,NextValue,NextAux>::Result Result;
};

/////////////////////////////////////////////////////


template<class SFrac, int Accuracy, class RetType = long double>
struct Compute;

template<class Numer, class Denom, int Accuracy, class RetType>
struct Compute<SRational<Numer,Denom>,Accuracy,RetType> {
  typedef SRational<Numer,Denom> Value;
  typedef typename RationalToDecimal<Value,Accuracy,DefaultDecimalBase>::Result TDec;
  typedef typename DoubleBase<typename TDec::Num>::Result BigInt;
  
  static RetType value() {
    return EvaluateToFloat<BigInt,RetType>::value()
         / DPow<DefaultDecimalBase,Accuracy,RetType>::value();
  }
};

template<int_t N, int Accuracy, class RetType>
struct Compute<SInt<N>,Accuracy,RetType> {
  typedef SInt<N> BigInt;
  static RetType value() { return static_cast<RetType>(N); }
};

template<class BI, int_t ND, base_t Base, int Accuracy, class RetType>
struct Compute<SDecimal<BI,ND,Base>,Accuracy,RetType> {
  typedef SDecimal<BI,ND,Base> Value;
  typedef typename Reduce<Value,Accuracy,Base>::Result TDec;
  typedef typename DoubleBase<typename TDec::Num>::Result BigInt;
  
  static RetType value() {
    return EvaluateToFloat<BigInt,RetType>::value() 
           / DPow<Base,Accuracy,RetType>::value();
  }
};

template<int_t N, int_t ND, base_t Base, int Accuracy, class RetType>
struct Compute<SDecimal<SInt<N>,ND,Base>,Accuracy,RetType> {
  typedef SDecimal<SInt<N>,ND,Base> Value;
  
  static RetType value() {
    return static_cast<RetType>(N) 
           / DPow<Base,ND,RetType>::value();
  }
};


////////////////////////////////////////////////////////

template<class T>
struct Cout;

template<int_t N>
struct Cout<SInt<N> > 
{
  static void apply(std::ostream& os) { 
    os << N;
  }
};

template<bool S, class H, class T, base_t Base>
struct Cout<SBigInt<S,Loki::Typelist<H,T>,Base> > 
{
  static const int_t W = NDigits<Base-1,10>::value;
  typedef Cout<SBigInt<S,T,Base> > Next;
  
  static void apply(std::ostream& os) { 
    Next::apply(os);
    os.fill('0');
    os.width(W);
    os << std::right << H::value << " ";
  }
};

template<bool S, class H, base_t Base>
struct Cout<SBigInt<S,Loki::Typelist<H,Loki::NullType>,Base> > {
  static void apply(std::ostream& os) { 
//     os << H::Value << " ";
    if (!S)
      os << "-";
    os << H::value << " ";
  }
};

template<class N, class D>
struct Cout<SRational<N,D> > 
{
  typedef Cout<N> CN;
  typedef Cout<D> CD;
  
  static void apply(std::ostream& os) { 
    CN::apply(os);
    os << " / ";
    CD::apply(os);
  }
};

//////////////////////////////////////////

template<bool S, class H, class T, base_t Base, int_t NDecPlaces, base_t DecBase>
struct Cout<SDecimal<SBigInt<S,Loki::Typelist<H,T>,Base>,NDecPlaces,DecBase> >
{
  static const int_t W = NDigits<Base-1,10>::value;
  static const int_t DW = NDigits<DecBase-1,10>::value;
  static const int_t Len = NL::Length<SBigInt<S,Loki::Typelist<H,T>,Base> >::value;
  static const int_t DP = DW * NDecPlaces;
  typedef Cout<SDecimal<SBigInt<S,T,Base>,NDecPlaces,DecBase> > Next;
  
  static void apply(std::ostream& os, const int_t len = 0) { 
    Next::apply(os,len+W);
    os.fill('0');
    if (DP < len+W && DP > len) {
      int_t d = 1;
      for (int i = 0; i < DP-len; ++i) d *= 10;
      os.width(W-DP+len);
      os << std::right << H::value/d << "." << H::value%d;
    }
    else {
      os.width(W);
      os << std::right << H::value;
    }
    if (DP == len)
      os << ".";
  }
};

template<bool S, class H, base_t Base, int_t NDecPlaces, base_t DecBase>
struct Cout<SDecimal<SBigInt<S,Loki::Typelist<H,Loki::NullType>,Base>,NDecPlaces,DecBase> > 
{
  static const int_t HW = NDigits<H::value,10>::value;
  static const int_t DP = NDigits<DecBase-1,10>::value * NDecPlaces;
  
  static void apply(std::ostream& os, const int_t len = 0) 
  { 
    if (!S)
      os << "-";
    if (DP >= len+HW) {
      os << "0.";
      os.fill('0');
      os.width(DP-len);
      os << std::right << H::value;
    }
    else if (DP < len+HW && DP > len) {
      int_t d = 1;
      for (int i = 0; i < DP-len; ++i) d *= 10;
      os << H::value/d << "." << H::value%d;
    }
    else
      os << H::value;
    if (DP == len)
      os << ".";
  }
};

template<int_t N, int_t NDecPlaces, base_t DecBase>
struct Cout<SDecimal<SInt<N>,NDecPlaces,DecBase> > 
{
  static const bool S = (N>=0);
  static const int_t AN = S ? N : -N;
  static const int_t HW = NDigits<AN,10>::value;
  
  static void apply(std::ostream& os, const int_t len = 0) 
  { 
    if (!S)
      os << "-";
    if (NDecPlaces >= len+HW) {
      os << "0.";
      os.fill('0');
      os.width(NDecPlaces-len);
      os << std::right << AN;
    }
    else if (NDecPlaces < len+HW && NDecPlaces > len) {
      int_t d = 1;
      for (int i = 0; i < NDecPlaces-len; ++i) d *= 10;
      os << AN/d << "." << AN%d;
    }
    else
      os << AN;
  }
};

} // namespace MF

#endif /*__metafunc_h*/
