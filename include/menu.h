/* -*- C++ -*- */
#pragma once

// A menu system with minimalistic assumptions
// Rui Azevedo - Mar2019
// ruihfazevedo@gmail.com

struct Nil {};

// menu system trait /////////////////////////////////////////*
/*
still we need a way of including types between every type in composition
to allow counter-part initializers
ex: default space usage info when geometry constrains are used on drawing...
1-include in base api?
  - we might not need it, it it will be inconditional!
2-injected by used core
  - not device dependent, its ok we need it when we need it
  => SysDef must include the core => Core must not be partameterized by SysDef
  solution: build core in two fases, the inner can include the SysDef
  or let the core build the SysDef (defer params)
  .. still the inner can not receive incomplete core
3-composed by the core
  - this can generate many versions of the core, not good!
4-use type taint?
  - not on return types, we will lose the info
  - virtuals would create a lot of vtables!
*/

template<typename RawOut,typename Src=Nil,typename idx_t=int>
struct SysDef {
  using Interface=struct IfPrompt;
  using RawOutput=RawOut;
  using Source=Src;
  using Idx=idx_t;

  enum Roles:uint16_t {
    noRole=0,
    panelRole=1,
    titleRole=2,
    bodyRole=4,
    itemRole=8,
    accelRole=16,
    cursorRole=32,
    modeRole=64,
    valueRole=128,
    unitRole=256,
    menuRole=512,
  };

  template<Roles m,typename O>
  struct Role:public O {
    using O::O;
    static constexpr Roles mask=m;
  };

  template<typename O> using asPanel=Role<panelRole,O>;
  template<typename O> using asTitle=Role<titleRole,O>;
  template<typename O> using asBody=Role<bodyRole,O>;
  template<typename O> using asPrompt=Role<itemRole,O>;
  template<typename O> using asAccel=Role<accelRole,O>;
  template<typename O> using asCursor=Role<cursorRole,O>;
  template<typename O> using asMode=Role<modeRole,O>;
  template<typename O> using asUnit=Role<unitRole,O>;
  template<typename O> using asMenu=Role<menuRole,O>;

  ////////////////////////////////////////////////////////////
  // the interface ///////////////////////////////////////
  struct IfPrompt {
    inline virtual size_t sz() const {return 0;}
    inline virtual IfPrompt& operator[](size_t) {return *this;}
    // inline virtual void set(size_t i,IfPrompt& o) {return O::get(n);}
    inline virtual RawOut& operator<<(RawOut& o) {return o;}
  };

  // interface adapter for typelevel compositions
  template<typename O>
  struct Prompt:public IfPrompt,public O {
    using Type=O;//get sub type
    using O::O;
    template<typename ... V>
    Prompt(V ... v):O(v...) {}
    template<typename ... V>
    Prompt(const char* o,V ... v):O(o,v...) {}
    inline size_t sz() const override {return O::sz();}
    inline IfPrompt& operator[](size_t i) override {return O::get(i);}
    // inline void set(size_t i,IfPrompt& o) override {return O::set(i,o);}
    inline RawOut& operator<<(RawOut& o) override {return O::print(o);}
  };

  /////////////////////////////////////////////////////////////////////
  // static mixins ///////////////////////////////////

  //static interface terminal (with an user cargo)
  struct Item:public Src {
    inline size_t sz() const {return 0;}
    inline IfPrompt& get(size_t) {return *(Prompt<Item>*)this;}
    // inline IfPrompt& set(size_t i,IfPrompt& o) {throw 1;}
    static inline RawOut& print(RawOut& o) {return o;}
    inline IfPrompt& operator[](size_t i) {return get(i);}
  };

  // default static parts --------------
  // for efficient composition (type level)
  // this can be extended outside here as this is an open data type
  template<typename O=Item>
  class Text:public virtual O {
  protected:
    const char* text;
  public:
    inline Text(const char* text):text(text) {}
    inline RawOut& print(RawOut& o) {o<<text;return O::print(o);}
  };

  template<typename O=Item>
  struct Static {
    template<const char**text>
    struct Text:public virtual O {
      static inline RawOut& print(RawOut& o) {o<<text[0];return O::print(o);}
    };
    template<idx_t n>
    class Menu:public virtual O {
    protected:
      IfPrompt* data[n];
    public:
      template<typename ... V>
      Menu(V ... v):data{v...} {}
      template<typename ... V>
      Menu(const char* o,V ... v):O(o),data{v...} {}
      // Menu(const char* o,initializer_list<IfPrompt*> v):O(o),data(v) {}
      inline size_t sz() const {return n;}
      inline IfPrompt& get(size_t i) {return *data[i];}
      inline IfPrompt& operator[](size_t i) {return get(i);}
    };
  };

};

template<typename RawOut,typename idx_t=int>
inline RawOut& operator<<(RawOut& o, typename SysDef<RawOut,Nil,idx_t>::IfPrompt& i) {return i.operator<<(o);}