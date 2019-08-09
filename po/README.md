
## To add a new translation

1. Add the new language code to `LINGUAS`
2. Run `./configure` in the top-level directory (you may also need to run `./autogen.sh` beforehand)
3. Run `make amsynth.pot` from this `po` directory
4. Create a new `.po` file using `amsynth.pot` as the starting point and edit with a tool such as [Gtranslator](https://wiki.gnome.org/Apps/Gtranslator)
5. Do a `make install` and run amsynth to check that the translations appear as expected
6. Create a pull request containing both files

## Testing translations

You can run amsynth in a different language using the following syntax;

    LANGUAGE=fr ./amsynth

## To update the translation files

After adding, removing or changing the translatable text in the source code, the translation files must be updated.

1. Review `POTFILES.in` and add or remove source files as necessary
2. Run `./configure` in the top-level directory (you may also need to run `./autogen.sh` beforehand)
3. Run `make amsynth.pot` from this `po` directory
4. Run `make update-po` from this `po` directory
5. Review and edit the `.po` files as necessary
6. Commit the updated `.pot` and `.po` files
