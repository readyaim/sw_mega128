
1���������ط�֧ local_branch
   git branch local_branch

2���������ط�֧local_branch ���л���local_branch��֧
   git checkout -b local_branch

3���л�����֧local_branch
    git checkout local_branch

4�����ͱ��ط�֧local_branch��Զ�̷�֧ remote_branch������������ϵ
      a.Զ������remote_branch��֧�����Ѿ��������ط�֧local_branch�ұ����Ѿ��л���local_branch
          git push
     b.Զ������remote_branch��֧��δ�������ط�֧local_branch�ұ����Ѿ��л���local_branch
         git push -u origin/remote_branch
     c.Զ��û����remote_branch��֧���������Ѿ��л���local_branch
        git push origin local_branch:remote_branch

5��ɾ�����ط�֧local_branch
      git branch -d local_branch

6��ɾ��Զ�̷�֧remote_branch
     git push origin  :remote_branch
     git branch -m | -M oldbranch newbranch ��������֧�����newbranch���ַ�֧�Ѿ����ڣ�����Ҫʹ��-Mǿ��������������ʹ��-m������������
   git branch -d | -D branchname ɾ��branchname��֧
   git branch -d -r branchname ɾ��Զ��branchname��֧

7���鿴���ط�֧
      git branch

8���鿴Զ�̺ͱ��ط�֧
      git branch -a
-------------------------
#���ڴ�remoteȡ��֧
git branch -a               //check remote branch
git fetch origin            //get all branch from remote
git fetch origin branch1
git fetch origin branch1:branch2    //get remote branch1 to local branch2
git fetch remote_repo remote_branch_name:local_branch_name
----------------------------
## works better
## ignore all files
*
## ignore all folders
!/**/
# include all *.c files in root and subfolders
!*.c
!*.h
!*.hex
#exclude folders
/example/
# include output/*.hex
#!sw/output/*.hex
# include .gitignore
!.gitignore
# include *.py in all folders(subfolders)
#!sw
#!/sw/*.c
_______________
## ignore all files
*
## ignore all folders
!/**/
# include output/*.hex
!output/*.hex
# include .gitignore
!.gitignore
# include *.py in all folders(subfolders)
!*.c
!.h
________________
1. generate username
    git config --global user.name "rea*im"

2. generate email
    git config --global user.email "sha*un1*8@outlook.com"

3. generate key
    ssh-keygen -t rsa -b 4096 -C "sha*un1*8@outlook.com"

4. copy key
    clip < ~/.ssh/id_rsa.pub

5. paste key
    login github, add the key to SSH in profile

6. Test (attention: git@github, not rea*im@gihub)
    ssh -T git@github.com
:Hi rea*im! You've successfully authenticated, but GitHub does not provide shell access.
success!!!

7. clone(download) for new(1st) time:
    a. git init
    b. git clone git@github.com:readyaim/TB_formula.git

8. upload for first time
    a. "start a project" in github.com
    b. git init
    c .git add .
    d. git commit -m "new"
    e. git remote add origin git@github.com:readyaim/AVR_test_128A.git
    f. git push -u origin master

9. download for 2nd time or after
    git remote add origin git@github.com:readyaim/TB_formula.git
    git pull origin master
    
10. revert a file (or files)
    git checkout head name.c
    git checkout head *.c
    
    
--------------------------------------
��or create a new repository on the command line

echo "# try1" >> README.md
git init
git add README.md
git commit -m "first commit"
git remote add origin git@github.com:readyaim/try1.git
git push -u origin master
��or push an existing repository from the command line

git remote add origin git@github.com:readyaim/try1.git
git push -u origin master
��or import code from another repository
You can initialize this repository with code from a Subversion, Mercurial, or TFS project.
g
Import code
------------------------------
�鿴add��index��֮ǰindex�Ĳ��
git diff --cached
-----------------------------
unstage file������add���work directory��repo�����仯
git reset HEAD~n filename
--------------------------
add+commit
git commit -a -m
-----------------------------
wd<-index<-repo
git checkout head filename
--------------------------
