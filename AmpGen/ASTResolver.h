#include <memory.h>
#include <bits/stdint-uintn.h>
#include <stddef.h>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <future>

#include "AmpGen/Expression.h"
#include "AmpGen/CacheTransfer.h"
#include "AmpGen/Spline.h"

namespace AmpGen {
  class MinuitParameter;
  class MinuitParameterSet;

  /// \class ASTResolver
  /// Traverses trees in IExpression::resolveDependencies()
  /// to keep track of the dependencies of the tree
  /// such as sub-trees, external parameters and the event type map.
  
  class ASTResolver 
  { 
    public: 
      ASTResolver(const std::map<std::string, size_t>& evtMap = {} , 
          const MinuitParameterSet* mps = nullptr );
      bool hasSubExpressions() const;
      
      void reduceSubTrees(); 
      void cleanup();
      void getOrderedSubExpressions( Expression& expression, 
          std::vector< std::pair<size_t,Expression>>& dependentSubexpressions );

      template <class TYPE> void resolve( const TYPE& obj ){}

      template <class TYPE, class ...ARGS> size_t addCacheFunction( const std::string& name, ARGS&... args )
      {
        auto it = m_cacheFunctions.find(name);
        if( it != m_cacheFunctions.end() ) return it->second->address();
        auto cacheFunction = std::make_shared<TYPE>(nParameters, args... );
        m_cacheFunctions[name] = cacheFunction;
        nParameters += cacheFunction->size();
        return nParameters - cacheFunction->size();
      }
      size_t nParams() const { return nParameters ; }       
      bool enableCuda() const { return enable_cuda ; }
      bool enableCompileConstants() const { return enable_compileTimeConstants ;} 
      std::map<std::string, std::shared_ptr<CacheTransfer>> cacheFunctions() const;
      void addResolvedParameter(const IExpression* param, const std::string& thing);
      void addResolvedParameter(const IExpression* param, const size_t& address, const size_t& arg=0);
      std::string resolvedParameter( const IExpression* param ) const; 

    private: 
      std::map<const IExpression* , std::string> m_resolvedParameters; 
      std::map<std::string, std::shared_ptr<CacheTransfer>> m_cacheFunctions;
      std::map<std::string, size_t>      evtMap;                  /// event specification 
      std::map<std::string, std::string> parameterMapping;        /// Mapping of parameters to compile parameters
      const MinuitParameterSet*          mps;                     /// Set of MinuitParameters 
      std::map<const SubTree*, uint64_t> tempTrees;               /// temporary store of sub-trees for performing cse reduction 
      std::map<uint64_t, Expression>     subTrees;                /// Unordered sub-trees 
      unsigned int                       nParameters;             /// Number of parameters
      bool                               enable_cuda;             /// flag to generate CUDA code <<experimental>>
      bool                               enable_compileTimeConstants; /// flag to enable compile time constants <<experimental>>
    
  };
  
  template <> void ASTResolver::resolve<Parameter>( const Parameter& obj );
  template <> void ASTResolver::resolve<SubTree>  ( const SubTree  & obj );
  template <> void ASTResolver::resolve<Spline>   ( const Spline   & obj );
}
