#include "BulletSystem"
#include "ShapeSystem"

#include <QtEntity/EntityManager>

BulletSystem::BulletSystem()
    : QtEntity::SimpleEntitySystem(Bullet::staticMetaObject)
{
    QTE_ADD_PROPERTY("movement", QPoint, Bullet, movement, setMovement);
}


void BulletSystem::step(int frameNumber, int totalTime, int delta)
{
   // float t = float(totalTime) / 1000.0f;

    for(auto i = begin(); i != end(); ++i)
    {
        /*Enemy* enemy = qobject_cast<Enemy*>(i.value());
        float lifetime = float(enemy->lifetime()) / 1000.0f;
        enemy->setLifetime(enemy->lifetime() + delta);
        Shape* shape;
        _entityManager->component(i.key(), shape);
        int x = sin(t / 10.0f + lifetime) * 300 + 280;
        int y = float(lifetime * 3.0f) + cos(t / 20.0f) * 10 - 50;
        shape->setPosition(QPoint(x, y));*/
    }
}
