/* CLDR unicode extractor. See http://cldr.unicode.org/ instead.
 - rlyeh, public domain

# CLDR
Compact data from the Unicode Common Locale Data Repository

For anyone interested, I just dumped most of the CLDR data in a compact way (see [provided CLDR.INI file](cldr.ini)).

The final data for all languages is 13,727,497 bytes, but still highly compressible, as seen below.

```lisp
BSC:    1,023,137 bytes, ratio=92.5468% enctime=1211822us dectime=757387us 
BROTLI: 1,287,148 bytes, ratio=90.6236% enctime=67236011us dectime=50243us 
LZMA25: 1,369,212 bytes, ratio=90.0258% enctime=3961970us dectime=98609us 
LZIP:   1,369,811 bytes, ratio=90.0214% enctime=3895218us dectime=131528us 
LZMA20: 1,423,667 bytes, ratio=89.6291% enctime=3334007us dectime=103905us 
MINIZ:  1,892,977 bytes, ratio=86.2103% enctime=697077us dectime=31830us 
ZSTD:   2,108,053 bytes, ratio=84.6436% enctime=65694us dectime=34525us 
LZ4HC:  2,151,652 bytes, ratio=84.326% enctime=491871us dectime=13641us 
LZ4:    2,918,991 bytes, ratio=78.7362% enctime=37851us dectime=13775us 
RAW:   13,727,497 bytes, ratio=0% enctime=16242us dectime=7658us 
```

This is what is currently processed from the CLDR repos:
- [ ] skipped
- [x] extracted

cldr-core/supplemental/
- [ ] aliases.json
- [ ] calendarData.json
- [ ] calendarPreferenceData.json
- [x] characterFallbacks.json
- [ ] codeMappings.json
- [x] currencyData.json
- [x] gender.json
- [ ] languageData.json
- [ ] languageMatching.json
- [ ] likelySubtags.json
- [x] measurementData.json
- [ ] metaZones.json
- [ ] numberingSystems.json
- [x] ordinals.json
- [x] parentLocales.json
- [x] plurals.json
- [ ] primaryZones.json
- [ ] references.json
- [x] telephoneCodeData.json
- [ ] territoryContainment.json
- [ ] territoryInfo.json (interesting!)
- [x] timeData.json
- [x] weekData.json
- [x] windowsZones.json

cldr-dates-modern\main\xx-XX
- [ ] ca-generic.json
- [x] ca-gregorian.json
- [x] dateFields.json
- [x] timeZoneNames.json

cldr-localenames-modern\main\xx-XX
- [x] languages.json
- [ ] localeDisplayNames.json
- [ ] scripts.json
- [x] territories.json
- [ ] transformNames.json
- [ ] variants.json

cldr-misc-modern\main\xx-XX
- [x] characters.json
- [x] contextTransforms.json
- [x] delimiters.json
- [x] layout.json
- [x] listPatterns.json
- [ ] posix.json

cldr-numbers-modern\main\xx-XX
- [x] currencies.json
- [ ] numbers.json

cldr-segments-modern\segments\xx-XX
- [ ] suppressions.json

*/

#include <fstream>
#include <sstream>
#include <iostream>

#ifdef CLDR_BUILD_COMPRESSED
#include "bundle/bundle.hpp"
#endif CLDR_BUILD_COMPRESSED

#include "json11/json11.hpp"
#include "json11/json11.cpp"

#include "apathy/apathy.hpp"
#include "apathy/apathy.cpp"

bool flat( std::map<std::string,std::string> &map, const json11::Json &js, const std::string &path = std::string() ) {
    std::string sep = path.empty() ? "" : ".";
    for( auto &pair : js.object_items() ) {
        flat(map, pair.second, path + sep + pair.first );
    }
    unsigned c = 0;
    for( auto &item : js.array_items() ) {
        flat(map, item, path + sep + std::to_string(c++) );
    }
    if( !js.string_value().empty() ) {
        map[ path ] = js.string_value();
    }
    return true;
}

bool match( const char *str, const char *pattern ) {
    if( *pattern=='\0' ) return !*str;
    if( *pattern=='*' )  return match(str,pattern+1) || (*str && match(str+1,pattern));
    if( *pattern=='?' )  return *str && (*str != '.') && match(str+1,pattern+1);
    return (*str == *pattern) && match(str+1,pattern+1);
}

bool begins_with( const std::string &text, const std::string &start ) {
    if( text.size() < start.size() ) return false;
    return text.substr( 0, start.size() ) == start;
}
bool ends_with( const std::string &text, const std::string &end ) {
    if( text.size() < end.size() ) return false;
    return text.substr( text.size() - end.size() ) == end;
}

std::string replace( std::string self, const std::string &target, const std::string &replacement ) {
    size_t found = 0;
    while( ( found = self.find( target, found ) ) != std::string::npos ) {
        self.replace( found, target.length(), replacement );
        found += replacement.length();
    }
    return self;
}

#include <deque>

std::deque<std::string> tokenize( const std::string &self, const std::string &delimiters ) {
    std::string map( 256, '\0' );
    for( const unsigned char &ch : delimiters )
    map[ ch ] = '\1';
    std::deque< std::string > tokens(1);
    for( const unsigned char &ch : self ) {
        /**/ if( !map.at(ch)          ) tokens.back().push_back( char(ch) );
        else if( tokens.back().size() ) tokens.push_back( std::string() );
    }
    while( tokens.size() && !tokens.back().size() ) tokens.pop_back();
    return tokens;
}

std::string join( const std::deque<std::string> &vec, const std::string &sep ) {
    std::string joint, rep;
    for( auto &t : vec ) {
        joint += rep + t;
        rep = sep;
    }
    return joint;
}


int main( int argc, const char **argv ) {
    auto json = []( const std::string &file ) -> json11::Json {
        std::string data, err;
        std::ifstream ifs( file.c_str(), std::ios::binary );
        std::stringstream ss;
        ss << ifs.rdbuf();
        data = ss.str();
        auto json = json11::Json::parse(data, err);
        return json;
    };

    std::map<std::string, std::string> flatzilla;
    std::map<std::string, std::string> oop;
    std::map<std::string, std::string> ecs;

    auto insert = [&]( const std::string &field, const std::string &lang, const std::string &value ) {
        oop[ lang ] += field + "{" + value + "},";
        ecs[ field + "|" + value ] += lang + ",";
    };

    std::map< std::string, std::function<void(const json11::Json &)> > parsers;

    parsers["timeData.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["timeData"].object_items() ) {
            const auto lng = it.first;
            const auto fmt = it.second["_preferred"].string_value();
            insert("timeData", lng, fmt );
        }
    };

    parsers["weekData.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["weekData"].object_items() ) {
            const auto tag = it.first;
            for( auto &jt : it.second.object_items() ) {
                const auto lng = jt.first;
                const auto fmt = jt.second.string_value();
                insert("weekData", lng, fmt );
            }
        }
    };

    parsers["characterFallbacks.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["characters"]["character-fallback"].object_items() ) {
            const auto chr = it.first;
            const auto sub = it.second[0]["substitute"].string_value();
            insert("character-fallback", sub, chr );
        }
    };

    parsers["currencyData.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["currencyData"]["fractions"].object_items() ) {
            const auto cur = it.first;
            const auto rou = it.second["_rounding"].string_value();
            const auto dig = it.second["_digits"].string_value();
            insert("currencyData.fractions.rounding", cur, rou );
            insert("currencyData.fractions.digits", cur, dig );
        }
    };

    parsers["gender.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["gender"]["personList"].object_items() ) {
            const auto lng = it.first;
            const auto taint = it.second.string_value();
            insert("gender.personList", lng, taint );
        }
    };

    parsers["measurementData.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["measurementData"].object_items() ) {
            const auto tag = it.first;
            for( auto &jt : it.second.object_items() ) {
                const auto lng = jt.first;
                const auto prop = jt.second.string_value();
                insert("measurementData." + tag, lng, prop );
            }
        }
    };

    parsers["ordinals.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["plurals-type-ordinal"].object_items() ) {
            const auto lng = it.first;
            for( auto &jt : it.second.object_items() ) {
                const auto tag = jt.first;
                const auto prop = jt.second.string_value();
                insert( "plurals-type-ordinal." + tag.substr( tag.find_last_of("-") + 1 ), lng, prop.substr( 0, prop.find_first_of("@") - 1 ) );
            }
        }
    };

    parsers["plurals.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["plurals-type-cardinal"].object_items() ) {
            const auto lng = it.first;
            for( auto &jt : it.second.object_items() ) {
                const auto tag = jt.first;
                const auto prop = jt.second.string_value();
                insert( "plurals-type-cardinal." + tag.substr( tag.find_last_of("-") + 1 ), lng, prop.substr( 0, prop.find_first_of("@") - 1 ) );
            }
        }
    };

    parsers["parentLocales.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["parentLocales"]["parentLocale"].object_items() ) {
            const auto lng = it.first;
            const auto prop = it.second.string_value();
            insert("parentLocales.parentLocale", lng, prop );
        }
    };

    parsers["telephoneCodeData.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["telephoneCodeData"].object_items() ) {
            const auto lng = it.first;
            for( auto &jt : it.second.array_items() ) {
                const auto prop = jt["telephoneCountryCode"].string_value();
                insert("telephoneCodeData", lng, prop );
            }
        }
    };

    parsers["windowsZones.json"] = [&]( const json11::Json &js ) {
        //flat( flatzilla, js );
        for( auto &it : js["supplemental"]["windowsZones"]["mapTimezones"].array_items() ) {
            const auto lng = it["mapZone"]["_territory"].string_value();
            const auto prop = it["mapZone"]["_type"].string_value();
            insert("windowsZones.mapTimezones", lng, prop );
        }
    };

    parsers["ca-gregorian.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };
    parsers["dateFields.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };
    parsers["timeZoneNames.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };
    parsers["languages.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };
    parsers["territories.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };
    parsers["characters.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };
    parsers["contextTransforms.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };
    parsers["delimiters.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };
    parsers["layout.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };
    parsers["listPatterns.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };
    parsers["currencies.json"] = [&]( const json11::Json &js ) {
        flat( flatzilla, js );
    };

    std::cout << "Reading disk structure..." << std::endl;

    for( auto &file : apathy::lsr("*.json") ) {
        std::string basename = file.substr( file.find_last_of('/') + 1 );
        std::cout << "[ ] " << file << "...";
        if( auto parser = parsers[basename] ) {
            parser( json(file) );
            std::cout << " parsed\r[X]\n";
        } else {
            std::cout << " ignored\r[ ]\n";
        }
    }

    if(0) {
        std::cout << json11::Json( oop ).dump() << std::endl;
        std::cout << json11::Json( ecs ).dump() << std::endl;
        std::ofstream ofs1( "oop.dat", std::ios::binary );
        ofs1 << json11::Json( oop ).dump();
        std::ofstream ofs2( "ecs.dat", std::ios::binary );
        ofs2 << json11::Json( ecs ).dump();
        std::ofstream ofs3( "flatzilla.dat", std::ios::binary );
        ofs3 << json11::Json( flatzilla ).dump();
    }

    auto find_locales_1 = [&](const std::string &lgcd ) {
        std::string lang = lgcd.substr(0,2);
        std::string code = lgcd.substr(3);
        std::cout << lgcd << ":" << oop[ lgcd ] << std::endl;
        std::cout << lang << ":" << oop[ lang ] << std::endl;
        std::cout << code << ":" << oop[ code ] << std::endl;
    };

    auto find_locales_2 = [&](const std::string &lgcd ) {
        std::string lang = lgcd.substr(0,2);
        std::string code = lgcd.substr(3);
        for( auto &p : flatzilla ) {
            if( match(p.first.c_str(), (std::string() + "*." + lgcd + ".*" ).c_str())) {
                std::cout << p.first << "=" << p.second << std::endl;
            }
            else
            if( match(p.first.c_str(), (std::string() + "*." + lang + ".*" ).c_str())) {
                std::cout << p.first << "=" << p.second << std::endl;
            }
            else
            if( match(p.first.c_str(), (std::string() + "*." + code + ".*" ).c_str())) {
                std::cout << p.first << "=" << p.second << std::endl;
            }
        }
    };

    if(argc>1) { // examples "en-US", "es-US", "fr-FR", "fr-CA", "es-ES", ...
        find_locales_1(argv[1]);
        find_locales_2(argv[1]);
        return 0;
    }

    std::map<std::string,std::string> oopzilla;
    std::map<std::string,std::string> ecszilla;
    std::map<std::string,std::string> mapzilla;

    for( auto &p : flatzilla ) {
        auto split = ::tokenize( p.first, "." );
        if( split[0] == "main" ) {
            auto lang = split[1];
            split.pop_front();
            split.pop_front();
            auto tag = join(split, ".");
            oopzilla[ tag + "[" + lang + "]" ] = p.second;
            ecszilla[ tag + "|" + p.second  ] += lang + ",";
        }/*
        else
        if( split[0] == "supplemental" ) {
            auto lang = split[1];
            split.pop_front();
            auto tag = join(split, ".");
            mapzilla[ tag ] = p.second;
        }*/
    }

    for( auto &p : ecs ) {
        auto find = p.first.find_first_of('|');
        std::string id = p.first.substr( 0, find );
        std::string txt = p.first.substr( find + 1 );
        const std::string &langs = p.second;
        mapzilla[ id + "[" + langs.substr(0, langs.size()-1) + "]" ] = txt;
    }

    using shortcut = std::pair<std::string, std::string>;
    std::vector< shortcut > shortcuts;
    if(1) {
        //
        shortcuts.emplace_back( shortcut { "characters.", "chr." } );
        //
        shortcuts.emplace_back( shortcut { "contextTransforms.", "ctx." } );
        shortcuts.emplace_back( shortcut { ".stand-alone", ".standalone" } );
        //
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dateFormats.", "date.fmt." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.timeFormats.", "time.fmt." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dateTimeFormats.availableFormats.", "datetime.fmt." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dateTimeFormats.intervalFormats.", "datetime.fmt." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dateTimeFormats.appendItems.", "datetime.item." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dateTimeFormats.", "datetime.fmt." } );

        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dayPeriods.format.abbreviated.", "days.fmt.abbr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dayPeriods.format.narrow.", "days.fmt.narr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dayPeriods.format.wide.", "days.fmt.wide." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dayPeriods.standalone.abbreviated.", "days.abbr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dayPeriods.standalone.narrow.", "days.narr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.dayPeriods.standalone.wide.", "days.wide." } );

        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.days.format.abbreviated.", "day.fmt.abbr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.days.format.narrow.", "day.fmt.narr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.days.format.short.", "day.fmt.short." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.days.format.wide.", "day.fmt.wide." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.days.standalone.abbreviated.", "day.abbr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.days.standalone.narrow.", "day.narr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.days.standalone.short.", "day.short." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.days.standalone.wide.", "day.wide." } );

        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.eras.eraAbbr", "era.abbr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.eras.eraNames", "era.name." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.eras.eraNarrow", "era.narr." } );

        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.months.format.abbreviated.", "month.fmt.abbr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.months.format.narrow.", "month.fmt.narr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.months.format.wide.", "month.fmt.wide." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.months.standalone.abbreviated.", "month.abbr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.months.standalone.narrow.", "month.narr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.months.standalone.wide.", "month.wide." } );

        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.quarters.format.abbreviated.", "quarter.fmt.abbr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.quarters.format.narrow.", "quarter.fmt.narr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.quarters.format.wide.", "quarter.fmt.wide." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.quarters.standalone.abbreviated.", "quarter.abbr." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.quarters.standalone.narrow.", "quarter.narrow." } );
        shortcuts.emplace_back( shortcut { "dates.calendars.gregorian.quarters.standalone.wide.", "quarter.wide." } );

        shortcuts.emplace_back( shortcut { "dates.fields.", "" } );
        shortcuts.emplace_back( shortcut { "-narrow", ".narr" } );
        shortcuts.emplace_back( shortcut { "-short", ".shrt" } );
        shortcuts.emplace_back( shortcut { ".displayName", ".name" } );
        shortcuts.emplace_back( shortcut { ".relativeTime-type-future", ".next" } );
        shortcuts.emplace_back( shortcut { ".relativeTime-type-past", ".past" } );
        shortcuts.emplace_back( shortcut { ".relative-type-", ".rel." } );
        shortcuts.emplace_back( shortcut { ".relativeTimePattern-count-", "." } );

        shortcuts.emplace_back( shortcut { "dates.timeZoneNames.metazone.", "" } );
        shortcuts.emplace_back( shortcut { ".short.generic", ".gen" } );
        shortcuts.emplace_back( shortcut { ".short.standard", ".std" } );
        shortcuts.emplace_back( shortcut { ".short.daylight", ".dayl" } );
        shortcuts.emplace_back( shortcut { ".long.generic", ".gen.long" } );
        shortcuts.emplace_back( shortcut { ".long.standard", ".std.long" } );
        shortcuts.emplace_back( shortcut { ".long.daylight", ".dayl.long" } );

        shortcuts.emplace_back( shortcut { "dates.timeZoneNames.zone.", "" } );
        shortcuts.emplace_back( shortcut { "dates.timeZoneNames.", "zone." } );
        shortcuts.emplace_back( shortcut { ".regionFormat-type-daylight", ".dayl" } );
        shortcuts.emplace_back( shortcut { ".regionFormat-type-standard", ".std" } );
        shortcuts.emplace_back( shortcut { ".exemplarCity", ".city" } );

        shortcuts.emplace_back( shortcut { "listPatterns.listPattern-type-", "list." } );

        shortcuts.emplace_back( shortcut { "localeDisplayNames.languages.", "lang." } );
        shortcuts.emplace_back( shortcut { "localeDisplayNames.territories.", "code." } );

        shortcuts.emplace_back( shortcut { "numbers.currencies.", "$" } );
        shortcuts.emplace_back( shortcut { ".displayName", "" } );
        shortcuts.emplace_back( shortcut { "-count-", "." } );
        shortcuts.emplace_back( shortcut { ".symbol", "" } );
    }

    std::set<std::string> keys;

    for( auto &p : ecszilla ) {
        auto find = p.first.find_first_of('|');
        std::string id = p.first.substr( 0, find );
        std::string txt = p.first.substr( find + 1 );

        for( auto &sc : shortcuts ) {
            id = replace(id, sc.first, sc.second );
        }
        if( ends_with(id, ".city") ) {
            id.resize( id.size() - 5 );
            //id = "city." + id;
        }
        keys.insert(id);

        const std::string &langs = p.second;
        mapzilla[ id + "[" + langs.substr(0, langs.size()-1) + "]" ] = txt;
    }

    {
        std::ofstream ofs( "cldr.key", std::ios::binary );
        ofs << "; auto-generated by cldr.cc" << std::endl;
        ofs << "; rlyeh, public domain" << std::endl;
        ofs << std::endl;
        for(auto &s : keys) {
            ofs << s << std::endl;
        }
    }

    {
        std::ofstream ofs( "cldr.ini", std::ios::binary );
        ofs << "; auto-generated by cldr.cc" << std::endl;
        ofs << "; rlyeh, public domain" << std::endl;
        ofs << std::endl;
        for( auto &p : mapzilla ) {
            ofs << p.first << "=" << p.second << std::endl;
        }
    }

#ifdef CLDR_BUILD_COMPRESSED
    // check compressed sizes
    {
        std::ifstream ifs( "cldr.ini", std::ios::binary );
        std::stringstream ss; ss << ifs.rdbuf();
        std::string original = ss.str();

        using namespace bundle;
        std::vector<unsigned> encodings = { RAW, LZ4, LZ4HC, /*SHOCO,*/ MINIZ, LZMA20, LZIP, LZMA25, /*ZPAQ,*/ BROTLI, ZSTD, BSC };

        std::cout << "compressing CLDR.INI, " << original.size() << " bytes..." << std::endl;

        auto data = measures( original, encodings );
        for( auto &in : data ) {
            std::cout << in.str() << std::endl;
        }

        std::cout << "fastest decompressor: " << name_of( find_fastest_decompressor(data) ) << std::endl;
        std::cout << "fastest compressor: " << name_of( find_fastest_compressor(data) ) << std::endl;
        std::cout << "smallest compressor: " << name_of( find_smallest_compressor(data) ) << std::endl;
    }
#endif

    std::cout << "[x] done" << std::endl;
}
