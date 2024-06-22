// WARNING: DUE TO TEMPLATE PARAMETERS, THIS IS NOT A TRANSLATION FILE, IT IS AN IMPLEMENTATION INCLUDED DIRECTLY INTO THE HEADER, BoolExprTreeManager.h

#include "BoolExprTreeManager.h"



template<size_t ArrN>
char BoolExprTreeManager<ArrN>::AddGroup(const vector<size_t> &has, const vector<size_t> &hasNot,
                                         bool addNonImplicitlyIfImplicit, bool createNewTreeIfImplicit) {
    // Hueristic:
    // If we can add as explicit: DO IT!
    // If we cannot add, make a new tree
    // If we can add as implicit, MORE COMPLICATED, MORE COMPLEX USE AND MANAGEMENT!!!!!
    // Trees maintain themselves independently...
    // If we add it implicitly, components it adds should not be added as explicits NOR should those component arrays be affect by the group

    BoolExprBitVector<ArrN> newGroupExpr = MakeFromSpecVec<ArrN>(has, hasNot);

    // Get trees with components
    unordered_set<size_t> treesWithExplComponents;
    vector<size_t> componentsWithoutTrees;
    PRINT("WITH TAGS: ");
    BoolExprBitVector<ArrN> bd{ _tags, _tags };
    DEBUG_Print_Boolean_Expr(bd);
    for (size_t h : has) {
        // If the component is a TAG, then we don't need to consider it for only having a single explicit tree
        if (_tags.at(h / (sizeof(size_t) * CHAR_BIT)) & ((size_t)1 << (h % (sizeof(size_t) * CHAR_BIT)))) continue;

        bool compHasTree = false;
        PRINT("*** TRYING TO PARSE " << h << std::endl);
        for (size_t i = 0; i < _exprTrees.size(); i++) {
            if (_exprTrees[i]->hasAsExplicitComponent(h)) {
                treesWithExplComponents.insert(i);
                compHasTree = true;
            }
        }
        if (!compHasTree) {
            componentsWithoutTrees.push_back(h);
        }
    }


    // If no trees use ANY of its components, make a new tree
    if (treesWithExplComponents.empty()) {
        PRINT("Making new TREE" << std::endl);
        _exprTrees.emplace_back(new BoolExprTree<ArrN>());
        _exprTrees.back()->Add(newGroupExpr, _tags); // Guarenteed to be explicit add
        return EXPLICIT_ADD;
    }
        // If there is a single tree which can use ALL of its components
    else if (treesWithExplComponents.size() == 1) {

        PRINT("Adding to Tree" << std::endl);
        BoolExprTree<ArrN>* tree = _exprTrees[*treesWithExplComponents.begin()].get();
        if (tree->CouldBeAdded(newGroupExpr))
            return tree->Add(newGroupExpr, _tags);
        else return FAILURE_ADD;
    }
        // If there are multiple trees which use its components OR a single tree which uses only SOME
    else {
        char result = FAILURE_ADD;
        // Add the implicit groups IMPLICITLY to all the trees we possible can
        if (addNonImplicitlyIfImplicit) {
            for (size_t t : treesWithExplComponents) {
                PRINT("Scattered group, scattering! Implicit add for {");
                BoolExprTree<ArrN>* tree = _exprTrees[t].get();
                for (auto a : tree->_explicitAffectedComponents)
                    PRINT(a << ",");
                PRINT("}  ");
                if (tree->CouldBeAdded(newGroupExpr)) {
                    PRINT(" SUCCESS!" << std::endl);
                    result = IMPLICIT_ADD;
                    tree->AddImplicit(newGroupExpr);
                }
                else {
                    PRINT(" FAILURE!" << std::endl);
                }
            }
        }
        // Create a new tree which we can add the group to for the components which can take it (with them explicit at the root)
        if (createNewTreeIfImplicit && !componentsWithoutTrees.empty()) {
            BoolExprBitVector<ArrN> newTreeRootExpr = MakeFromSpecVec<ArrN>(componentsWithoutTrees, {});
            PRINT("Making NEW TREE for an IMPLICIT/PARTIAL GROUP {");
            for (auto a : componentsWithoutTrees) PRINT(a << ", ");
            PRINT("}" << std::endl);

            _exprTrees.emplace_back(new BoolExprTree<ArrN>());
            _exprTrees.back()->Add(newTreeRootExpr); // Guarenteed to be explicit add
            _exprTrees.back()->AddImplicit(newGroupExpr); // This NEEDS to be implicit
            return IMPLICIT_ADD;

        } else
            return result;
    }
}