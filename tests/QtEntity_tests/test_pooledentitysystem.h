#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtEntity/EntityManager>
#include <QtEntity/SimpleEntitySystem>
#include <QtEntity/PooledEntitySystem>
#include <QElapsedTimer>
#include "common.h"

using namespace QtEntity;

class TestingSystemPooled : public PooledEntitySystem<Testing>
{
public:
    TestingSystemPooled(EntityManager* em, size_t capacity = 0, size_t chunkSize = 4)
        : PooledEntitySystem<Testing>(em, capacity, chunkSize)
    {
    }

    virtual QVariantMap toVariantMap(QtEntity::EntityId eid, int context = 0) override
    {
        Q_UNUSED(context)
        QVariantMap m;
        Testing* t;
        if(component(eid, t))
        {
            m["myint"]     = t->myInt();         
        }
        return m;    
    }

    virtual void fromVariantMap(QtEntity::EntityId eid, const QVariantMap& m, int context = 0) override
    {
        Q_UNUSED(context)
        Testing* t;
        if(component(eid, t))
        {
            if(m.contains("myint"))     t->setMyInt(m["myint"].toInt());
        }
    }
};


class PooledEntitySystemTest: public QObject
{
    Q_OBJECT


#define NUM_COMPONENTS 500
#define NUM_ITERATIONS 10000

private:
    template <typename T>
    int speed1(T& es)
    {
        QElapsedTimer timer;
        timer.start();
        QtEntity::EntityId id = 1;
        for(int i = 1; i <= NUM_COMPONENTS; ++i)
        {
            es.createComponent(id++);
        }

        for(int i = 1; i <= NUM_COMPONENTS; i += 4)
        {
            es.destroyComponent(i);
        }

        for(int i = 1; i <= NUM_COMPONENTS; ++i)
        {
            es.createComponent(id++);
        }

        for(int i = 0; i <= NUM_ITERATIONS; ++i)
        {
            auto end = es.end();
            for(auto j = es.begin(); j != end; ++j)
            {
                void* c = j->second;
                Testing* t = static_cast<Testing*>(c);
                t->setMyInt(t->myInt() + 1);
            }
        }

        return timer.elapsed();
    }

    int speed2(EntitySystem& es)
    {

        QElapsedTimer timer;
        timer.start();

        QtEntity::EntityId id = 1;
        for(int i = 1; i <= NUM_COMPONENTS; ++i)
        {
            es.createComponent(id++);
        }

        for(int i = 1; i <= NUM_COMPONENTS; i += 4)
        {
            es.destroyComponent(i);
        }

        for(int i = 1; i <= NUM_COMPONENTS; ++i)
        {
            es.createComponent(id++);
        }

        for(int i = 0; i <= NUM_ITERATIONS; ++i)
        {
            auto end = es.pend();
            for(auto j = es.pbegin(); j != end; ++j)
            {
                void* c = *j;
                Testing* t = static_cast<Testing*>(c);
                t->setMyInt(t->myInt() + 1);
            }
        }

        return timer.elapsed();
    }

private slots:

    void createAndFetch()
    {
        EntityManager em;
        TestingSystemPooled* ts = new TestingSystemPooled(&em);

		QVariantMap m;
		m["myint"] = 666;
        void* c = ts->createComponent(1, m);

        void* c2 = ts->component(1);
        QVERIFY(c == c2);
        QVERIFY(ts->component(1) != nullptr);

        void* c3 = ts->component(2);
        QVERIFY(c3 == nullptr);

        Testing* tr = static_cast<Testing*>(c);
        QVERIFY(tr != nullptr);
        QCOMPARE(tr->myInt(), 666);

    }

    void reserve()
    {
        EntityManager em;
        TestingSystemPooled* ts = new TestingSystemPooled(&em, 0, 2);
		for(int i = 0; i < 3; ++i) ts->createComponent(i + 1);
        QCOMPARE(ts->count(), (size_t)3);
        QCOMPARE(ts->capacity(), (size_t)4);
    }

    void destroyOne()
    {
        EntityManager em;
        TestingSystemPooled* ts = new TestingSystemPooled(&em, 0, 2);
		ts->createComponent(1);
        ts->destroyComponent(1);
        QCOMPARE(ts->count(), (size_t)0);
        QCOMPARE(ts->capacity(), (size_t)2);
    }


     void destroyMore()
    {
        EntityManager em;
        TestingSystemPooled* ts = new TestingSystemPooled(&em, 0, 2);
        QVariantMap m;
		m["myint"] = 1;		
        ts->createComponent(1, m);
        m["myint"] = 2;
        ts->createComponent(2, m);
        m["myint"] = 3;
        ts->createComponent(3, m);
        
        QCOMPARE(ts->count(), (size_t)3);
        ts->destroyComponent(2);
        QCOMPARE(ts->count(), (size_t)2);

        QCOMPARE(static_cast<Testing*>(ts->component(1))->myInt(), 1);
        QCOMPARE(static_cast<Testing*>(ts->component(3))->myInt(), 3);
        ts->destroyComponent(1);
        ts->destroyComponent(3);
        QCOMPARE(ts->count(), (size_t)0);
    }


     void erase()
    {
        EntityManager em;
        TestingSystemPooled* ts = new TestingSystemPooled(&em);
        QVariantMap m;
		m["myint"] = 1;
        ts->createComponent(1, m);
        m["myint"] = 2;
        ts->createComponent(2, m);
        m["myint"] = 3;
        ts->createComponent(3, m);
        auto i = ts->begin(); 
        QCOMPARE(i->second->myInt(), 1);
        ++i;
        QCOMPARE(i->second->myInt(), 2);
        i = ts->erase(i);
        QCOMPARE(i->second->myInt(), 3);
        QVERIFY(ts->component(2) == nullptr);
    }

     void clear()
    {
        EntityManager em;
        TestingSystemPooled* ts = new TestingSystemPooled(&em);   
        ts->createComponent(1);
        ts->createComponent(2);        
        ts->createComponent(3);
        ts->clear();
        QCOMPARE(ts->count(), (size_t)0);
        ts->createComponent(1);
        QCOMPARE(ts->count(), (size_t)1);
    }
    
    void speedTest()
    {
        {
            EntityManager em;
            TestingSystemPooled* pooled = new TestingSystemPooled(&em, 0, 8);
            TestingSystem* simple = new TestingSystem(&em);

            float timepooled = float(speed1(*pooled)) / 1000.0f;
            float timesimple = float(speed1(*simple)) / 1000.0f;
            qDebug() << "Time pooled1: " << timepooled;
            qDebug() << "Time simple1: " << timesimple;
        }

        {
            EntityManager em;
            TestingSystemPooled* pooled = new TestingSystemPooled(&em, 0, 8);
            TestingSystem* simple = new TestingSystem(&em);

            float timepooled = float(speed2(*pooled)) / 1000.0f;
            float timesimple = float(speed2(*simple)) / 1000.0f;
            qDebug() << "Time pooled1: " << timepooled;
             qDebug() << "Time simple1: " << timesimple;
        }
    }

    void iteratorTest1()
    {
        EntityManager em;
        TestingSystemPooled* pooled = new TestingSystemPooled(&em, 0, 8);
        QVERIFY(pooled->begin() == pooled->end());

        QVariantMap m;
		m["myint"] = 1; pooled->createComponent(1, m);
        m["myint"] = 2; pooled->createComponent(2, m);
        m["myint"] = 3; pooled->createComponent(3, m);
        TestingSystemPooled::iterator i = pooled->begin();
        
        Testing* t = i->second;
        QCOMPARE(t, static_cast<Testing*>(pooled->component(1)));
        QCOMPARE((++i)->second, static_cast<Testing*>(pooled->component(2)));
        QCOMPARE((++i)->second, static_cast<Testing*>(pooled->component(3)));
        TestingSystemPooled::iterator j = ++i;
        TestingSystemPooled::iterator end = pooled->end();
        QCOMPARE(j, end);
    }

    void iteratorTest2()
    {
        EntityManager em;
        TestingSystemPooled* pooled = new TestingSystemPooled(&em, 0, 8);
        QVariantMap m;
        for(int i = 1; i <= 10; ++i)
        {
            m["myint"] = i;
            pooled->createComponent(i, m);
        }
        int count = 1;
        for(TestingSystemPooled::iterator i = pooled->begin(); i != pooled->end(); ++i)
        {
            Testing* o = i->second;
            QCOMPARE(o, static_cast<Testing*>(pooled->component(count++)));
        }
    }

    void iteratorTest3()
    {
        EntityManager em;
        TestingSystemPooled* pooled = new TestingSystemPooled(&em, 0, 8);
        QVariantMap m;
        for(int i = 1; i <= 10; ++i)
        {
            m["myint"] = i;
            pooled->createComponent(i, m);
        }
        int count = 1;
        for(PIterator i = pooled->pbegin(); i != pooled->pend(); ++i)
        {
            Testing* o = static_cast<Testing*>(*i);
            QCOMPARE(o, static_cast<Testing*>(pooled->component(count++)));
        }
    }
};
