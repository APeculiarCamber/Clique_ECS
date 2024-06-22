#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <random>

namespace Rate {
    struct Position {
        float x, y, z;
    };

    struct Velocity {
        float x, y, z;
    };

    struct Acceleration {
        float x, y, z;
    };

    struct Jerk {
        float x, y, z;
    };

    struct Snap {
        float x, y, z;
    };

    struct Crackle {
        float x, y, z;
    };

    struct Pop {
        float x, y, z;
    };

    struct Lock {
        float x, y, z;
    };

    struct Drop {
        float x, y, z;
    };

    struct Tumble {
        float x, y, z;
    };

    struct Waver {
        float x, y, z;
    };

    struct Quiver {
        float x, y, z;
    };

    class BaseChanger {
    public:
        virtual size_t ID() = 0;
        virtual void Update(float dt) = 0;
    };

    // TODO : this might not actually be the ideal case to show ECS but it does allow me to attempt to polish speeds...
    class PositionBC : public BaseChanger {
    public:
        Position pos{1, 1, 1};

        void Update(float dt) override {
            // Do nothing - position doesn't change
        }

        size_t  ID() {
            return 0;
        }
    };

    class VelocityBC : public BaseChanger {
    public:
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };

        void Update(float dt) override {
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 1;
        }
    };

    class AccelerationBC : public BaseChanger {
    public:
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };

        void Update(float dt) override {
            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            vel.z += acc.z * dt;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 2;
        }
    };

    class JerkBC : public BaseChanger {
    public:
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jerk{ 1, 1, 1 };

        void Update(float dt) override {
            acc.x += jerk.x * dt;
            acc.y += jerk.y * dt;
            acc.z += jerk.z * dt;

            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            vel.z += acc.z * dt;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 3;
        }
    };

    class SnapBC : public BaseChanger {
    public:
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jerk{ 1, 1, 1 };
        Snap snap{ 1, 1, 1 };

        void Update(float dt) override {
            jerk.x += snap.x * dt;
            jerk.y += snap.y * dt;
            jerk.z += snap.z * dt;

            acc.x += jerk.x * dt;
            acc.y += jerk.y * dt;
            acc.z += jerk.z * dt;

            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            vel.z += acc.z * dt;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 4;
        }
    };

    class CrackleBC : public BaseChanger {
    public:
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jerk{ 1, 1, 1 };
        Snap snap{ 1, 1, 1 };
        Crackle crackle{ 1, 1, 1 };

        void Update(float dt) override {
            snap.x += crackle.x * dt;
            snap.y += crackle.y * dt;
            snap.z += crackle.z * dt;

            jerk.x += snap.x * dt;
            jerk.y += snap.y * dt;
            jerk.z += snap.z * dt;

            acc.x += jerk.x * dt;
            acc.y += jerk.y * dt;
            acc.z += jerk.z * dt;

            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            vel.z += acc.z * dt;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 5;
        }
    };

    class PopBC : public BaseChanger {
    public:
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jerk{ 1, 1, 1 };
        Snap snap{ 1, 1, 1 };
        Crackle crackle{ 1, 1, 1 };
        Pop pop{ 1, 1, 1 };

        void Update(float dt) override {
            crackle.x += pop.x * dt;
            crackle.y += pop.y * dt;
            crackle.z += pop.z * dt;

            snap.x += crackle.x * dt;
            snap.y += crackle.y * dt;
            snap.z += crackle.z * dt;

            jerk.x += snap.x * dt;
            jerk.y += snap.y * dt;
            jerk.z += snap.z * dt;

            acc.x += jerk.x * dt;
            acc.y += jerk.y * dt;
            acc.z += jerk.z * dt;

            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            vel.z += acc.z * dt;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 6;
        }
    };

    class LockBC : public BaseChanger {
    public:
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jerk{ 1, 1, 1 };
        Snap snap{ 1, 1, 1 };
        Crackle crackle{ 1, 1, 1 };
        Pop pop{ 1, 1, 1 };
        Lock lck{ 1, 1, 1 };

        void Update(float dt) override {
            pop.x += lck.x * dt;
            pop.y += lck.y * dt;
            pop.z += lck.z * dt;

            crackle.x += pop.x * dt;
            crackle.y += pop.y * dt;
            crackle.z += pop.z * dt;

            snap.x += crackle.x * dt;
            snap.y += crackle.y * dt;
            snap.z += crackle.z * dt;

            jerk.x += snap.x * dt;
            jerk.y += snap.y * dt;
            jerk.z += snap.z * dt;

            acc.x += jerk.x * dt;
            acc.y += jerk.y * dt;
            acc.z += jerk.z * dt;

            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            vel.z += acc.z * dt;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 7;
        }
    };


    class DropBC : public BaseChanger {
    public:
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jerk{ 1, 1, 1 };
        Snap snap{ 1, 1, 1 };
        Crackle crackle{ 1, 1, 1 };
        Pop pop{ 1, 1, 1 };
        Lock lck{ 1, 1, 1 };
        Drop drp{ 1, 1, 1 };

        void Update(float dt) override {
            lck.x += drp.x * dt;
            lck.y += drp.y * dt;
            lck.z += drp.z * dt;

            pop.x += lck.x * dt;
            pop.y += lck.y * dt;
            pop.z += lck.z * dt;

            crackle.x += pop.x * dt;
            crackle.y += pop.y * dt;
            crackle.z += pop.z * dt;

            snap.x += crackle.x * dt;
            snap.y += crackle.y * dt;
            snap.z += crackle.z * dt;

            jerk.x += snap.x * dt;
            jerk.y += snap.y * dt;
            jerk.z += snap.z * dt;

            acc.x += jerk.x * dt;
            acc.y += jerk.y * dt;
            acc.z += jerk.z * dt;

            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            vel.z += acc.z * dt;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 8;
        }
    };

    class TumbleBC : public BaseChanger {
    public:
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jerk{ 1, 1, 1 };
        Snap snap{ 1, 1, 1 };
        Crackle crackle{ 1, 1, 1 };
        Pop pop{ 1, 1, 1 };
        Lock lck{ 1, 1, 1 };
        Drop drp{ 1, 1, 1 };
        Tumble tmbl{ 1, 1, 1 };

        void Update(float dt) override {
            drp.x += tmbl.x * dt;
            drp.y += tmbl.y * dt;
            drp.z += tmbl.z * dt;

            lck.x += drp.x * dt;
            lck.y += drp.y * dt;
            lck.z += drp.z * dt;

            pop.x += lck.x * dt;
            pop.y += lck.y * dt;
            pop.z += lck.z * dt;

            crackle.x += pop.x * dt;
            crackle.y += pop.y * dt;
            crackle.z += pop.z * dt;

            snap.x += crackle.x * dt;
            snap.y += crackle.y * dt;
            snap.z += crackle.z * dt;

            jerk.x += snap.x * dt;
            jerk.y += snap.y * dt;
            jerk.z += snap.z * dt;

            acc.x += jerk.x * dt;
            acc.y += jerk.y * dt;
            acc.z += jerk.z * dt;

            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            vel.z += acc.z * dt;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 9;
        }
    };


    class QuiverBC : public BaseChanger {
    public:
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jerk{ 1, 1, 1 };
        Snap snap{ 1, 1, 1 };
        Crackle crackle{ 1, 1, 1 };
        Pop pop{ 1, 1, 1 };
        Lock lck{ 1, 1, 1 };
        Drop drp{ 1, 1, 1 };
        Tumble tmbl{ 1, 1, 1 };
        Quiver qvr{ 1, 1, 1 };

        void Update(float dt) override {
            tmbl.x += qvr.x * dt;
            tmbl.y += qvr.y * dt;
            tmbl.z += qvr.z * dt;

            drp.x += tmbl.x * dt;
            drp.y += tmbl.y * dt;
            drp.z += tmbl.z * dt;

            lck.x += drp.x * dt;
            lck.y += drp.y * dt;
            lck.z += drp.z * dt;

            pop.x += lck.x * dt;
            pop.y += lck.y * dt;
            pop.z += lck.z * dt;

            crackle.x += pop.x * dt;
            crackle.y += pop.y * dt;
            crackle.z += pop.z * dt;

            snap.x += crackle.x * dt;
            snap.y += crackle.y * dt;
            snap.z += crackle.z * dt;

            jerk.x += snap.x * dt;
            jerk.y += snap.y * dt;
            jerk.z += snap.z * dt;

            acc.x += jerk.x * dt;
            acc.y += jerk.y * dt;
            acc.z += jerk.z * dt;

            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            vel.z += acc.z * dt;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 10;
        }
    };



    class WaverBC : public BaseChanger {
    public:
        Position pos{1, 1, 1};
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jerk{ 1, 1, 1 };
        Snap snap{ 1, 1, 1 };
        Crackle crackle{ 1, 1, 1 };
        Pop pop{ 1, 1, 1 };
        Lock lck{ 1, 1, 1 };
        Drop drp{ 1, 1, 1 };
        Tumble tmbl{ 1, 1, 1 };
        Quiver qvr{ 1, 1, 1 };
        Waver wvr{ 1, 1, 1 };

        void Update(float dt) override {
            qvr.x += wvr.x * dt;
            qvr.y += wvr.y * dt;
            qvr.z += wvr.z * dt;

            tmbl.x += qvr.x * dt;
            tmbl.y += qvr.y * dt;
            tmbl.z += qvr.z * dt;

            drp.x += tmbl.x * dt;
            drp.y += tmbl.y * dt;
            drp.z += tmbl.z * dt;

            lck.x += drp.x * dt;
            lck.y += drp.y * dt;
            lck.z += drp.z * dt;

            pop.x += lck.x * dt;
            pop.y += lck.y * dt;
            pop.z += lck.z * dt;

            crackle.x += pop.x * dt;
            crackle.y += pop.y * dt;
            crackle.z += pop.z * dt;

            snap.x += crackle.x * dt;
            snap.y += crackle.y * dt;
            snap.z += crackle.z * dt;

            jerk.x += snap.x * dt;
            jerk.y += snap.y * dt;
            jerk.z += snap.z * dt;

            acc.x += jerk.x * dt;
            acc.y += jerk.y * dt;
            acc.z += jerk.z * dt;

            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            vel.z += acc.z * dt;

            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        }

        size_t  ID() {
            return 11;
        }
    };

    /*
    Jerk (derivative of acceleration, rate at which acceleration changes)
    Snap (derivative of jerk, rate at which jerk changes)
    Crackle (derivative of snap, rate at which snap changes)
    Pop (derivative of crackle, rate at which crackle changes)
    Lock (derivative of pop, rate at which pop changes)
    Drop (derivative of lock, rate at which lock changes)
    Tumble (derivative of drop, rate at which drop changes)
    Waver (derivative of tumble, rate at which tumble changes)
    Quiver (derivative of waver, rate at which waver changes)
    */

    struct ChangeSharedResources {
        std::clock_t lastFrame;
        float deltaTime = 0;
    };


    void FillInVirtualVector(std::vector<BaseChanger*>& vec, const std::vector<size_t>& types, std::ostream& out) {

        std::clock_t start = std::clock();
        // TODO : pop these so we can benchmark them too
        for (size_t i = 0; i < types.size(); ++i) {
            switch (types[i]) {
            case 0:
                vec.push_back(new PositionBC());
                break;
            case 1:
                vec.push_back(new VelocityBC());
                break;
            case 2:
                vec.push_back(new AccelerationBC());
                break;
            case 3:
                vec.push_back(new JerkBC());
                break;
            case 4:
                vec.push_back(new SnapBC());
                break;
            case 5:
                vec.push_back(new CrackleBC());
                break;
            case 6:
                vec.push_back(new PopBC());
                break;
            case 7:
                vec.push_back(new DropBC());
                break;
            case 8:
                vec.push_back(new LockBC());
                break;
            case 9:
                vec.push_back(new TumbleBC());
                break;
            case 10:
                vec.push_back(new WaverBC());
                break;
            case 11:
                vec.push_back(new QuiverBC());
                break;
            }
        }
        std::clock_t end = std::clock();
        out << "'PTR_SETUP': " << (end - start) << ", " << std::endl;
    }
    template <typename Res, typename CompList>
    void FillInECS(ECS::Manager<Res, CompList>& manager, const std::vector<size_t>& types, std::ostream& out) {

        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jrk{ 1, 1, 1 };
        Snap snp{ 1, 1, 1 };
        Crackle crk{ 1, 1, 1 };
        Pop pop{ 1, 1, 1 };
        Drop drp{ 1, 1, 1 };
        Lock lck{ 1, 1, 1 };
        Tumble tmbl{ 1, 1, 1 };
        Waver wvr{ 1, 1, 1 };
        Quiver qvr{ 1, 1, 1 };

        std::clock_t start = std::clock();
        for (size_t i = 0; i < types.size(); ++i) {
            switch (types[i]) {
            case 0:
                manager.AddEntity(&pos);
                break;
            case 1:
                manager.AddEntity(&pos, &vel);
                break;
            case 2:
                manager.AddEntity(&pos, &vel, &acc);
                break;
            case 3:
                manager.AddEntity(&pos, &vel, &acc, &jrk);
                break;
            case 4:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp);
                break;
            case 5:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk);
                break;
            case 6:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop);
                break;
            case 7:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop, &drp);
                break;
            case 8:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop, &drp, &lck);
                break;
            case 9:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop, &drp, &lck, &tmbl);
                break;
            case 10:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop, &drp, &lck, &tmbl, &wvr);
                break;
            case 11:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop, &drp, &lck, &tmbl, &wvr, &qvr);
                break;
            default:
                manager.AddEntity(&pos); break;
            }
        }
        manager.CommitEntityChanges();

        std::clock_t end = std::clock();
        out << "'ECS_SETUP': " << (end - start) << ", " << std::endl;
    }
    template <typename Res, typename CompList>
    void FillInBoth(std::vector<BaseChanger*>& vec, ECS::Manager<Res, CompList>& manager, const std::vector<size_t>& types)
    {
        Position pos{ 1, 1, 1 };
        Velocity vel{ 1, 1, 1 };
        Acceleration acc{ 1, 1, 1 };
        Jerk jrk{ 1, 1, 1 };
        Snap snp{ 1, 1, 1 };
        Crackle crk{ 1, 1, 1 };
        Pop pop{ 1, 1, 1 };
        Drop drp{ 1, 1, 1 };
        Lock lck{ 1, 1, 1 };
        Tumble tmbl{ 1, 1, 1 };
        Waver wvr{ 1, 1, 1 };
        Quiver qvr{ 1, 1, 1 };

        for (size_t i = 0; i < types.size(); ++i) {

            switch (types[i]) {
            case 0:
                manager.AddEntity(&pos);
                vec.push_back(new PositionBC());
                break;
            case 1:
                manager.AddEntity(&pos, &vel);
                vec.push_back(new VelocityBC());
                break;
            case 2:
                manager.AddEntity(&pos, &vel, &acc);
                vec.push_back(new AccelerationBC());
                break;
            case 3:
                manager.AddEntity(&pos, &vel, &acc, &jrk);
                vec.push_back(new JerkBC());
                break;
            case 4:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp);
                vec.push_back(new SnapBC());
                break;
            case 5:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk);
                vec.push_back(new CrackleBC());
                break;
            case 6:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop);
                vec.push_back(new PopBC());
                break;
            case 7:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop, &drp);
                vec.push_back(new DropBC());
                break;
            case 8:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop, &drp, &lck);
                vec.push_back(new LockBC());
                break;
            case 9:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop, &drp, &lck, &tmbl);
                vec.push_back(new TumbleBC());
                break;
            case 10:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop, &drp, &lck, &tmbl, &wvr);
                vec.push_back(new WaverBC());
                break;
            case 11:
                manager.AddEntity(&pos, &vel, &acc, &jrk, &snp, &crk, &pop, &drp, &lck, &tmbl, &wvr, &qvr);
                vec.push_back(new QuiverBC());
                break;
            default:
                manager.AddEntity(&pos); break;
            }
        }
        manager.CommitEntityChanges();
    }


    template <typename A, typename B>
    /* __forceinline */ void AltApplyChangesRES(std::tuple<ChangeSharedResources*, A*, B*>& tup) {
        auto& a = std::get<A*>(tup);
        auto& b = std::get<B*>(tup);
        auto& res = std::get<ChangeSharedResources*>(tup);

        a->x += res->deltaTime * b->x;
        a->y += res->deltaTime * b->y;
        a->z += res->deltaTime * b->z;
    }


    template <typename A, typename B>
    /* __forceinline */ void ApplyChangesRES_NoTup(ChangeSharedResources* res, A* a, B* b) {
        a->x += res->deltaTime * b->x;
        a->y += res->deltaTime * b->y;
        a->z += res->deltaTime * b->z;
    }


    int RunChangeTimeBenchmark(size_t numEntities, size_t numItrs, size_t numDerivatives, bool useFulCascade, bool shufflePtrs, bool sortPtrs, std::ostream& out) {

        assert(numDerivatives >= 1 && numDerivatives <= 12);

        using Components = typename ECS::ComponentList<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop, Drop, Lock, Tumble, Waver, Quiver>;
        ECS::Manager<ChangeSharedResources, Components> manager;

        auto res = manager.GetSharedResources();
        res->lastFrame = std::clock();
        res->deltaTime = 0;

        // TODO : the system you have now will collapse on this, but you should think about if it actually NEEDS to...
        // TODO : static assert for the types made into groups, components, systems, etc. if it aint in the manager desc, we aint taking it.
        // I mean obviously we can fix this with system MUST HAVEs
        if (numDerivatives >= 2)
            manager.MakeGroup<GContains<Position, Velocity>, GNotContains<>>();
        if (numDerivatives >= 3)
            manager.MakeGroup<GContains<Position, Velocity, Acceleration>, GNotContains<>>();
        if (numDerivatives >= 4)
            manager.MakeGroup<GContains<Position, Velocity, Acceleration, Jerk>, GNotContains<>>();
        if (numDerivatives >= 5)
            manager.MakeGroup<GContains<Position, Velocity, Acceleration, Jerk, Snap>, GNotContains<>>();
        if (numDerivatives >= 6)
            manager.MakeGroup<GContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle>, GNotContains<>>();
        if (numDerivatives >= 7)
            manager.MakeGroup<GContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop>, GNotContains<>>();
        if (numDerivatives >= 8)
            manager.MakeGroup<GContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop, Lock>, GNotContains<>>();
        if (numDerivatives >= 9)
            manager.MakeGroup<GContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop, Lock, Drop>, GNotContains<>>();
        if (numDerivatives >= 10)
            manager.MakeGroup<GContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop, Lock, Drop, Tumble>, GNotContains<>>();
        if (numDerivatives >= 11)
            manager.MakeGroup<GContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop, Lock, Drop, Tumble, Waver>, GNotContains<>>();
        if (numDerivatives >= 12)
            manager.MakeGroup<GContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop, Lock, Drop, Tumble, Waver, Quiver>, GNotContains<>>();



        // SYSTEMS
        if (numDerivatives >= 12)
            manager.AddSystem<SContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop, Lock, Drop, Tumble>, SNotContains<>, AltApplyChangesRES<Waver, Quiver>>()->AssertFullEquiv();
        if (numDerivatives >= 11)
            manager.AddSystem<SContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop, Lock, Drop>, SNotContains<>, AltApplyChangesRES<Tumble, Waver>>()->AssertFullEquiv();
        if (numDerivatives >= 10)
            manager.AddSystem<SContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop, Lock>, SNotContains<>, AltApplyChangesRES<Drop, Tumble>>()->AssertFullEquiv();
        if (numDerivatives >= 9)
            manager.AddSystem<SContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle, Pop>, SNotContains<>, AltApplyChangesRES<Lock, Drop>>()->AssertFullEquiv();
        if (numDerivatives >= 8)
            manager.AddSystem<SContains<Position, Velocity, Acceleration, Jerk, Snap, Crackle>, SNotContains<>, AltApplyChangesRES<Pop, Lock>>()->AssertFullEquiv();
        if (numDerivatives >= 7)
            manager.AddSystem<SContains<Position, Velocity, Acceleration, Jerk, Snap>, SNotContains<>, AltApplyChangesRES<Crackle, Pop>>()->AssertFullEquiv();
        if (numDerivatives >= 6)
            manager.AddSystem<SContains<Position, Velocity, Acceleration, Jerk>, SNotContains<>, AltApplyChangesRES<Snap, Crackle>>()->AssertFullEquiv();
        if (numDerivatives >= 5)
            manager.AddSystem<SContains<Position, Velocity, Acceleration>, SNotContains<>, AltApplyChangesRES<Jerk, Snap>>()->AssertFullEquiv();
        if (numDerivatives >= 4)
            manager.AddSystem<SContains<Position, Velocity>, SNotContains<>, AltApplyChangesRES<Acceleration, Jerk>>()->AssertFullEquiv();
        if (numDerivatives >= 3)
            manager.AddSystem<SContains<Position>, SNotContains<>, AltApplyChangesRES<Velocity, Acceleration>>()->AssertFullEquiv();
        if (numDerivatives >= 2)
            manager.AddSystem<SContains<>, SNotContains<>, AltApplyChangesRES<Position, Velocity>>()->AssertFullEquiv();
        


        // SET UP THE ENTITIES AND CLASSES 
        std::vector<BaseChanger*> ptrPolyChanger;
        std::srand((unsigned int)std::time(nullptr));

        std::vector<size_t> randomDerivs;
        for (size_t i = 0; i < numEntities; ++i) {
            randomDerivs.push_back(std::abs(std::rand()) % numDerivatives);
        }

        out << "{" << std::endl;

        FillInECS(manager, randomDerivs, out);
        FillInVirtualVector(ptrPolyChanger, randomDerivs, out);
        //FillInBoth(ptrPolyChanger, manager, randomDerivs);


        manager.AddCleanupSystem([](ChangeSharedResources* res, decltype(manager)* manager) {
            std::clock_t endOfFrame = std::clock();
            res->deltaTime = (float)(endOfFrame - res->lastFrame) / (float)CLOCKS_PER_SEC;
            res->lastFrame = endOfFrame;
            });

        if (shufflePtrs)
            std::shuffle(ptrPolyChanger.begin(), ptrPolyChanger.end(), std::default_random_engine(time(nullptr)));
        if (sortPtrs)
            std::sort(ptrPolyChanger.begin(), ptrPolyChanger.end(), [](BaseChanger* p1, BaseChanger* p2) {return p1->ID() < p2->ID(); });

        if (!sortPtrs && !shufflePtrs) {
            manager.SetTickLimit(numItrs);
            clock_t ecsstart = std::clock();
            manager.RunSystems();
            clock_t ecsend = std::clock();
            out << "\'ECS\' : " << ((float)(ecsend - ecsstart) / (float)(CLOCKS_PER_SEC / 1000)) << ", " << std::endl;
        }
        clock_t start = std::clock();
        clock_t lastFrame = start;
        float dt = 0;
        for (size_t i = 0; i < numItrs; ++i) {
            for (size_t p = 0; p < ptrPolyChanger.size(); ++p) {
                ptrPolyChanger[p]->Update(dt);
            }

            std::clock_t endOfFrame = std::clock();
            dt = (float)(endOfFrame - lastFrame) / (float)CLOCKS_PER_SEC;
            lastFrame = endOfFrame;
        }
        clock_t end = std::clock();
        out << "\'PTR\' : " << ((float)(end - start) / (float)(CLOCKS_PER_SEC / 1000)) << std::endl << "}";
        //out << "'MEM_USAGE' : ";
        //out << "}";

        return 0;
    }
}

#endif