/*
Copyright (c) 2013 Martin Scheffler
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial 
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <QtEntityUtils/VariantManager>
#include <QtEntity/DataTypes>
#include <QTextStream>
#include <QMetaProperty>

namespace QtEntityUtils
{

    QtVariantProperty* VariantManager::addProperty(int propertyType, const QString &name)
    {
        if(propertyType == qMetaTypeId<float>())
        {
            propertyType = qMetaTypeId<double>();
        }
        return QtVariantPropertyManager::addProperty(propertyType, name);
    }

    
    int VariantManager::filePathTypeId()
    {
        return qMetaTypeId<QtEntityUtils::FilePath>();
    }


    int VariantManager::listId()
    {
        return QVariant::List;
    }


    bool VariantManager::isPropertyTypeSupported(int propertyType) const
    {
        if (propertyType == filePathTypeId() ||
            propertyType == listId())
        {
            return true;
        }
        return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
    }


    int VariantManager::valueType(int propertyType) const
    {
        if (propertyType == filePathTypeId())
        {
            return QVariant::String;
        }
        if (propertyType == listId())
        {
            return QVariant::List;
        }
        return QtVariantPropertyManager::valueType(propertyType);
    }


    QVariant VariantManager::value(const QtProperty *property) const
    {
        if(propertyType(property) == groupTypeId())
        {
            QList<QtProperty *> subs = property->subProperties();
            QVariantMap ret;
            for(auto i = subs.begin(); i != subs.end(); ++i)
            {
                ret[(*i)->propertyName()] = value(*i);
            }
            return ret;
        }
        if(propertyType(property) == listId())
        {
            QList<QtProperty *> subs = property->subProperties();
            QVariantList ret;
            for(auto i = subs.begin(); i != subs.end(); ++i)
            {
                ret.push_back(value(*i));
            }
            return ret;
        }
        if (_filePathValues.contains(property))
        {
            return QVariant::fromValue(_filePathValues[property].value);
        }        
        return QtVariantPropertyManager::value(property);
    }


    QStringList VariantManager::attributes(int propertyType) const
    {
        if (propertyType == filePathTypeId())
        {
            QStringList attr;
            attr << QLatin1String("filter");
            return attr;
        }

        if (propertyType == listId())
        {
            QStringList attr;
            attr << QLatin1String("prototypes");
            return attr;
        }
        return QtVariantPropertyManager::attributes(propertyType);
    }


    int VariantManager::attributeType(int propertyType, const QString &attribute) const
    {
        if (propertyType == filePathTypeId())
        {
            if (attribute == QLatin1String("filter"))
                return QVariant::String;
            return 0;
        }

        if (propertyType == listId())
        {
            if (attribute == QLatin1String("prototypes"))
               return QVariant::Map;
            return 0;
        }
        return QtVariantPropertyManager::attributeType(propertyType, attribute);
    }


    QVariant VariantManager::attributeValue(const QtProperty *property, const QString &attribute) const
    {
        if (_filePathValues.contains(property))
        {
            if (attribute == QLatin1String("filter"))
                return _filePathValues[property].filter;
            return QVariant();
        }
        if (_prototypeValues.contains(property) &&
                attribute == QLatin1String("prototypes"))
        {
            return _prototypeValues[property];
        }

        return QtVariantPropertyManager::attributeValue(property, attribute);
    }


    QString VariantManager::valueText(const QtProperty *property) const
    {
        if (_filePathValues.contains(property))
        {
            return _filePathValues[property].value;
        }        
        else
        {
            return QtVariantPropertyManager::valueText(property);
        }
    }


    void VariantManager::setValue(QtProperty *property, const QVariant &val)
    {
        if (_filePathValues.contains(property))
        {
            QString str = val.value<QtEntityUtils::FilePath>();
            FilePathData d = _filePathValues[property];
            if (d.value == str)
                return;
            d.value = str;
            _filePathValues[property] = d;
            emit propertyChanged(property);
            emit valueChanged(property, str);
            return;
        }
        QtVariantPropertyManager::setValue(property, val);
    }


    void VariantManager::setAttribute(QtProperty *property,
                    const QString &attribute, const QVariant &val)
    {
        if (_filePathValues.contains(property))
        {
            if (attribute == QLatin1String("filter")) {
                if (val.type() != QVariant::String && !val.canConvert(QVariant::String))
                    return;
                QString str = val.value<QString>();
                FilePathData d = _filePathValues[property];
                if (d.filter == str)
                    return;
                d.filter = str;
                _filePathValues[property] = d;
                emit attributeChanged(property, attribute, str);
            }
            return;
        }

        if(_prototypeValues.contains(property) && attribute == QLatin1String("prototypes"))
        {
            _prototypeValues[property] = val.toMap();
            return;
        }

        QtVariantPropertyManager::setAttribute(property, attribute, val);
    }


    void VariantManager::initializeProperty(QtProperty *property)
    {
        int t = propertyType(property);
        if (t == filePathTypeId())
            _filePathValues[property] = FilePathData();
        if (t == listId())
            _prototypeValues[property] = QVariantMap();
        QtVariantPropertyManager::initializeProperty(property);
    }


    void VariantManager::uninitializeProperty(QtProperty *property)
    {
        _filePathValues.remove(property);
        _prototypeValues.remove(property);
        QtVariantPropertyManager::uninitializeProperty(property);
    }

}
