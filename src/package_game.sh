ECHO "Copying Files..."

COPY Dev:NEONnoir/NEONnoir TO RAM:NEONnoir/

COPY Dev:NEONnoir/NEONnoir.info TO RAM:NEONnoir/

COPY Dev:NEONnoir/data TO RAM:NEONnoir/data ALL


ECHO "Archiving..."
lha -a -r -x a RAM:NEONnoir.lha  RAM:NEONnoir


ECHO "Delete destination..."
DELETE Dev:NEONnoir/build ALL


ECHO "Create destination..."
MAKEDIR Dev:NEONnoir/build


ECHO "Copy archive to destination..."
COPY RAM:NEONnoir.lha TO Dev:NEONnoir/build

