#include <QtEntity/SimpleEntitySystem>
#include <QDebug>
#include <QMetaProperty>
#include <stdexcept>

namespace QtEntity
{

    class SimpleVIterator : public EntitySystem::VIterator
    {
    public:

        SimpleVIterator(SimpleEntitySystem::ComponentStore::Iterator it)
            : _iter(it)
        {
        }

        virtual VIterator* clone()
        {
            return new SimpleVIterator(_iter);
        }

        virtual QObject* operator*() 
        {
            return *_iter;
        }

        virtual QObject* operator->() 
        {
            return *_iter;
        }

        virtual bool equal(VIterator* other) 
        {
            Q_ASSERT(typeid(other) == typeid(SimpleVIterator));
            return static_cast<const SimpleVIterator*>(other)->_iter == _iter;
        }

        virtual void increment() 
        {
            ++_iter;
        }

    private:
        SimpleEntitySystem::ComponentStore::iterator _iter;
    };

    SimpleEntitySystem::SimpleEntitySystem(const QMetaObject& componentMeta)
        : _entityManager(NULL)
        , _componentMetaObject(&componentMeta)
    {
    }


    SimpleEntitySystem::~SimpleEntitySystem()
	{
        foreach(QObject* c, _components)
        {
            delete c;
        }
	}


    QObject* SimpleEntitySystem::getComponent(EntityId id) const
    {
        auto i = _components.find(id);
        if(i == _components.end())
        {
            return nullptr;
        }
        return i.value();
    }


    bool SimpleEntitySystem::hasComponent(EntityId id) const
    {
        return (this->getComponent(id) != nullptr);
    }


    QObject* SimpleEntitySystem::createComponent(EntityId id, const QVariantMap& propertyVals)
    {
        // check if component already exists
        if(getComponent(id) != nullptr)
        {
            throw std::runtime_error("Component already existss!");
        }

        // use QMetaObject to construct new instance
        QObject* obj = this->createObjectInstance(id, propertyVals);

        // out of memory?
        if(obj == nullptr)
        {
            qCritical() << "Could not construct component. Have you declared a default constructor with Q_INVOKABLE?";
            throw std::runtime_error("Component could not be constructed.");
        }

        // store
        _components[id] = obj;

        emit componentCreated(id, obj);
        return obj;
    }


    bool SimpleEntitySystem::destroyComponent(EntityId id)
    {        
        auto i = _components.find(id);
        if(i == _components.end()) return false;
        emit componentAboutToDestruct(id, i.value());
        delete i.value();
        _components.erase(i);
        return true;
    }


    const QMetaObject& SimpleEntitySystem::componentMetaObject() const
    {
        return *_componentMetaObject;
    }


    QObject* SimpleEntitySystem::createObjectInstance(EntityId id, const QVariantMap& propertyVals)
    {
        QObject* obj = _componentMetaObject->newInstance();
        applyParameters(obj, propertyVals);
        return obj;
    }


    void SimpleEntitySystem::applyParameters(QObject* obj, const QVariantMap& properties)
    {
        if(properties.empty()) return;

        const QMetaObject* meta = obj->metaObject();
        for(int i = 0; i < meta->propertyCount(); ++i)
        {
            QMetaProperty prop = meta->property(i);

            if(!prop.isWritable())
            {
                qWarning() << "Trying to initialize a non-writable property. Name is: " << prop.name();
            }
            else
            {
                auto var = properties.find(prop.name());
                if(var != properties.end())
                {
                    bool success = prop.write(obj, var.value());
                    if(!success)
                    {
                        qWarning() << "Could not set property. Name is: " << prop.name();
                    }
                }
            }
        }
    }

    
    size_t SimpleEntitySystem::count() const
    {
        return _components.size();
    }

      
    QObject* SimpleEntitySystem::componentAt(size_t at)
    {
        auto it = _components.begin();
        std::advance(it, at);
        return *it;
    }


    EntitySystem::Iterator SimpleEntitySystem::pbegin()
    {
        return EntitySystem::Iterator(new SimpleVIterator(begin()));
    }


    EntitySystem::Iterator SimpleEntitySystem::pend()
    {
        return EntitySystem::Iterator(new SimpleVIterator(end()));
    }
}
