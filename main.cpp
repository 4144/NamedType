#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include "named_type.hpp"

// Usage examples

namespace 
{
namespace test
{

/*
template<typename T>
decltype(auto) tee(T&& value)
{
    std::cout << value << '\n';
    return std::forward<T>(value);
}
*/

using Meter = fluent::NamedType<double, struct MeterParameter, fluent::Addable, fluent::Comparable>;
Meter operator"" _meter(unsigned long long value) { return Meter(value); }
//Meter operator"" _meter(long double value) { return Meter(value); }

using Kilometer = fluent::MultipleOf<Meter, std::ratio<1000>>;
Kilometer operator"" _kilometer(unsigned long long value) { return Kilometer(value); }
Kilometer operator"" _kilometer(long double value) { return Kilometer(value); }

using Millimeter = fluent::MultipleOf<Meter, std::milli>;

using Centimeter = fluent::MultipleOf<Millimeter, std::ratio<10>>;

using Width = fluent::NamedType<Meter, struct WidthParameter>;
using Height = fluent::NamedType<Meter, struct HeightParameter>;

class Rectangle
{
public:
    Rectangle(Width width, Height height) : width_(width.get()), height_(height.get()) {}
    Meter getWidth() const { return width_; }
    Meter getHeight() const { return height_; }

private:
    Meter width_;
    Meter height_;
};

bool testBasicUsage()
{
    Rectangle r(Width(10_meter), Height(12_meter));
    return r.getWidth().get() == 10 &&  r.getHeight().get() == 12;
}

using NameRef = fluent::NamedType<std::string&, struct NameRefParameter>;

void changeValue(const NameRef name)
{
    name.get() = "value2";
}

bool testReference()
{
    std::string value = "value1";
    changeValue(NameRef(value));
    return value == "value2";
}

template<typename Function>
using Comparator = fluent::NamedType<Function, struct ComparatorParameter>;

template <typename Function>
std::string performAction(Comparator<Function> comp)
{
    return comp.get()();
}

bool testGenericType()
{
    return performAction(fluent::make_named<Comparator>([](){ return std::string("compare"); })) == "compare";
}

double distanceInKilometer(Kilometer d)
{
    return d.get();
}

bool testMeterToKm()
{
    return distanceInKilometer(31000_meter) == 31;
}

double distanceInMeter(Meter d)
{
    return d.get();
}

bool testKmToMeter()
{
    return distanceInMeter(31_kilometer) == 31000;
}

double distanceInMillimeter(Millimeter d)
{
    return d.get();
}

bool testKmToMillimeter()
{
    return distanceInMillimeter(31_kilometer) == 31000000;
}

bool testCmToMeter()
{
    return distanceInMeter(Centimeter(31)) == 0.31;
}

bool testMeterToKmWithDecimals()
{
    return distanceInKilometer(31234_meter) == 31.234;
}

bool testComparable()
{
    return 10_meter == 10_meter && !(10_meter == 11_meter)
        && !(10_meter == 11_meter) && 10_meter !=  11_meter;
}
    
bool testConvertible()
{
    using MyInt = fluent::NamedType<int, struct MyIntTag, fluent::ImplicitlyConvertible<unsigned>::templ>;
    MyInt myInt(-1);
    int uMyInt = myInt;
    return uMyInt == std::numeric_limits<unsigned>::max();
}

bool testAddableComparableConvertible()
{
    return 1_kilometer + 200_meter == 1200_meter
        && 1_kilometer + 200_meter == 1.2_kilometer;
}

struct ConvertMileFromAndToKilometer
{
    static double convertFrom(double kilometer) { return kilometer / 1.609; }
    static double convertTo(double mile) { return mile * 1.609; }
};

using Mile = fluent::ConvertibleTo<Kilometer, ConvertMileFromAndToKilometer>;
Mile operator"" _mile(unsigned long long mile) { return Mile(mile); }

bool testMileToKm()
{
    return distanceInKilometer(2_mile) == 2 * 1.609;
}

bool testMileToMeter()
{
    return distanceInMeter(2_mile) == 2 * 1000 * 1.609;
}

double distanceInMile(Mile d)
{
    return d.get();
}

bool testKmToMile()
{
    return distanceInMile(2_kilometer) == 2 / 1.609;
}

using Watt = fluent::NamedType<double, struct WattTag>;
Watt operator"" _watt(unsigned long long watt) { return Watt(watt); }

struct ConvertDBFromAndToWatt
{
    static double convertFrom(double watt) { return 10 * log(watt) / log(10); }
    static double convertTo(double db) { return pow(10, db / 10); }
};
using dB = fluent::ConvertibleTo<Watt, ConvertDBFromAndToWatt>;
dB operator"" _dB(long double db) { return dB(db); }

double powerInDb(dB power)
{
    return power.get();
}

bool testWattToDb()
{
    return std::abs(powerInDb(230_watt) - 23.617) < 10e-2;
}

double powerInWatt(Watt power)
{
    return power.get();
}

bool testDbToWatt()
{
    return std::abs(powerInWatt(25.6_dB) - 363.078) < 10e-2;
}

bool testHash()
{
    using SerialNumber = fluent::NamedType<std::string, struct SerialNumberTag, fluent::Comparable, fluent::Hashable>;

    std::unordered_map<SerialNumber, int> hashMap = { {SerialNumber{"AA11"}, 10}, {SerialNumber{"BB22"}, 20} };
    SerialNumber cc33{"CC33"};
    hashMap[cc33] = 30;
    return hashMap[SerialNumber{"AA11"}] == 10
        && hashMap[SerialNumber{"BB22"}] == 20
        && hashMap[cc33] == 30;
}

template <typename TestFunction>
bool launchTest(std::string const& testName, TestFunction testFunction)
{
    const bool success = testFunction();
    if (!success)
        std::cout << "Test - " << testName << ": FAILED\n";
    return success;
}

void launchTests()
{
    bool success = true;
    success &= launchTest("Basic usage", testBasicUsage);
    success &= launchTest("Passing by reference", testReference);
    success &= launchTest("Generic type", testGenericType);
    success &= launchTest("meter to km", testMeterToKm);
    success &= launchTest("km to meter", testKmToMeter);
    success &= launchTest("km to mm", testKmToMillimeter);
    success &= launchTest("cm to m", testCmToMeter);
    success &= launchTest("mile to km", testMileToKm);
    success &= launchTest("km to mile", testKmToMile);
    success &= launchTest("mile to meter", testMileToMeter);
    success &= launchTest("dB to watt", testDbToWatt);
    success &= launchTest("watt to dB", testWattToDb);
    success &= launchTest("meter to km with decimals", testMeterToKmWithDecimals);
    success &= launchTest("comparable", testComparable);
    success &= launchTest("convertible", testConvertible);
    success &= launchTest("addable comparable convertible", testAddableComparableConvertible);
    success &= launchTest("hash", testHash);
    if (success)
        std::cout << "All tests PASSED\n";
}

}
}
int main()
{
    test::launchTests();
}
