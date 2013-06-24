TEMPLATE = subdirs

defineTest(addSubdirs) {
        for(subdirs, 1) {
                entries = $$files($$subdirs)
                for(entry, entries) {
                        name = $$replace(entry, [/\\\\], _)
                        subdir_path = $$replace(entry, \\\\, /)
                        SUBDIRS += $$name
                        eval ($${name}.subdir = $$subdir_path)
                        for(dep, 2):eval ($${name}.depends += $$replace(dep, [/\\\\], _))
                        export ($${name}.subdir)
                        export ($${name}.depends)
                }
        }
        export (SUBDIRS)
}

addSubdirs(lib)
addSubdirs(test, lib)
