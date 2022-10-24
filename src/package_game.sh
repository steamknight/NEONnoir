ECHO "Copying Files..."

COPY Develop:NEONnoir/neonnoir TO RAM:NEONnoir/

COPY Develop:NEONnoir/neonnoir.info TO RAM:NEONnoir/

COPY Develop:NEONnoir/data TO RAM:NEONnoir/data ALL


ECHO "Archiving..."
lha -a -r -x a RAM:NEONnoir.lha  RAM:NEONnoir


ECHO "Delete destination..."
DELETE Develop:NEONnoir/build ALL


ECHO "Create destination..."
MAKEDIR Develop:NEONnoir/build


ECHO "Copy archive to destination..."
COPY RAM:NEONnoir.lha TO Develop:NEONnoir/build

