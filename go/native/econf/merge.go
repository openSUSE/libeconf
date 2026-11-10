package econf

// setAt writes a copy of e at index idx of *fe, growing the slice as
// needed, and interns e's group into dest. This mirrors cpy_file_entry
// (which interns into dest_kf as part of copying) plus the realloc'd C
// array writes in mergefiles.c.
func setAt(dest *File, fe *[]entry, idx int, e entry) {
	dest.internGroup(e.group)
	for len(*fe) <= idx {
		*fe = append(*fe, entry{})
	}
	(*fe)[idx] = e
}

// insertNoGroup mirrors mergefiles.c's insert_nogroup.
func insertNoGroup(dest *File, fe *[]entry, etc *File) int {
	n := 0
	for n < len(etc.entries) && etc.entries[n].group == noGroup {
		dest.internGroup(etc.entries[n].group)
		*fe = append(*fe, etc.entries[n])
		n++
	}
	return n
}

// mergeExistingGroups mirrors mergefiles.c's merge_existing_groups: usr's
// entries are copied through as-is, group by group; for each usr group,
// matching etc entries (by group) either overwrite the value of an
// existing key (etc wins) or get appended as a new key at the end of that
// group's block.
func mergeExistingGroups(dest *File, fe *[]entry, uf, ef *File, etcStart int) int {
	mergeLength := etcStart
	tmp := etcStart
	addedKeys := etcStart

	uLen := len(uf.entries)
	eLen := len(ef.entries)

	for i := 0; i <= uLen; i++ {
		boundary := (i == uLen && i > 0) ||
			(i > 0 && i < uLen && uf.entries[i].group != uf.entries[i-1].group)
		if boundary {
			for j := etcStart; j < eLen; j++ {
				if uf.entries[i-1].group != ef.entries[j].group {
					continue
				}
				newKey := true
				for k := mergeLength; k < i+tmp; k++ {
					if (*fe)[k].key == ef.entries[j].key {
						v := valueOf(ef.entries[j].value)
						(*fe)[k].value = &v
						newKey = false
						break
					}
				}
				if newKey {
					setAt(dest, fe, i+addedKeys, ef.entries[j])
					addedKeys++
				}
			}
			mergeLength = i + addedKeys
			tmp = addedKeys
		}
		if i != uLen {
			setAt(dest, fe, i+addedKeys, uf.entries[i])
		}
	}
	return mergeLength
}

// addNewGroups mirrors mergefiles.c's add_new_groups: groups that exist
// only in etc (and aren't the no-group sentinel) are appended wholesale.
func addNewGroups(dest *File, fe *[]entry, uf, ef *File, mergeLength int) int {
	addedKeys := mergeLength
	for i := range ef.entries {
		if ef.entries[i].group == noGroup {
			continue
		}
		newGroup := true
		for j := range uf.entries {
			if uf.entries[j].group == ef.entries[i].group {
				newGroup = false
				break
			}
		}
		if newGroup {
			setAt(dest, fe, addedKeys, ef.entries[i])
			addedKeys++
		}
	}
	if addedKeys > 0 {
		*fe = (*fe)[:addedKeys]
	}
	return addedKeys
}

// MergeFiles merges etcFile on top of usrFile; entries in etcFile win.
func MergeFiles(usrFile, etcFile *File) (*File, error) {
	if usrFile == nil || etcFile == nil {
		return nil, ErrArgumentIsNullValue
	}

	merged := &File{delimiter: usrFile.delimiter, comment: usrFile.comment}
	if len(etcFile.entries)+len(usrFile.entries) == 0 {
		return merged, nil
	}

	etcStartsNoGroup := len(etcFile.entries) == 0 || etcFile.entries[0].group == noGroup
	usrStartsNoGroup := len(usrFile.entries) > 0 && usrFile.entries[0].group == noGroup

	var fe []entry
	etcStart := 0
	if etcStartsNoGroup && !usrStartsNoGroup {
		etcStart = insertNoGroup(merged, &fe, etcFile)
	}
	mergeLength := mergeExistingGroups(merged, &fe, usrFile, etcFile, etcStart)
	mergeLength = addNewGroups(merged, &fe, usrFile, etcFile, mergeLength)

	merged.entries = fe[:mergeLength]
	return merged, nil
}
